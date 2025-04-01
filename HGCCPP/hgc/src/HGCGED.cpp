#include "HGCGED.h"

#pragma region helper functions

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCDFAInspection"
// prints a debug message to the console
void debugLog(const std::string& message) {
	std::cout << "Debug Log: " + message << std::endl;
}
#pragma clang diagnostic pop

// prints an info message to the console
void showInfo(const std::string& message) {
	std::cout << "Info: " + message << std::endl;
}

// prints a warning to the console
void showWarning(const std::string& message) {
	std::cout << "Warning! " + message << std::endl;
}

// throws an error with a given console output
void throwError(const std::string& message, const std::string& details) {
	throw std::runtime_error("Error! " + message + " " + details);
}

// filters out the argument "--threads" of the method arguments and returns its passed value, e.g. "--threads 10" returns 10.
int parseMethodThread(const std::string& methodArgumentsString) {
	int value = -1;
	for (std::size_t index = 0; index < methodArgumentsString.size(); index++) {
		if (methodArgumentsString.at(index) == '-') {
			if (index + 1 < methodArgumentsString.size() && methodArgumentsString.at(index + 1) == '-') {
				std::size_t length = std::string("threads ").size();
				if (index + 1 + length < methodArgumentsString.size() && methodArgumentsString.substr(index + 2, length) == "threads ") {
					std::string valueString;
					for (std::size_t slicedIndex = index + 2 + length; slicedIndex < methodArgumentsString.size(); slicedIndex++) {
						if (methodArgumentsString.at(slicedIndex) == ' ')
							break;
						valueString += methodArgumentsString.at(slicedIndex);
					}
					value = std::stoi(valueString);
				}
			}
		}
	}
	return value;
}

// parses the init type string into the ged init type enum
ged::Options::InitType parseInitType(const std::string& initTypeString) {
	if (initTypeString.empty()) {
		showInfo("No initialization type passed. Defaulting to \"LAZY\".");
		return ged::Options::InitType::LAZY_WITHOUT_SHUFFLED_COPIES;
	}
	else if (initTypeString == "LAZY")
		return ged::Options::InitType::LAZY_WITHOUT_SHUFFLED_COPIES;
	else if (initTypeString == "EAGER")
		return ged::Options::InitType::EAGER_WITHOUT_SHUFFLED_COPIES;
	else
		throwError("Couldn't construct HGC Environment:", "\"" + initTypeString + "\" is an invalid initialization type.");
}

// parses the method string into the ged method
ged::Options::GEDMethod HGCGED::loadMethod(const std::string& methodString) {
	if (methodString.empty()) {
		showInfo("No GED method passed. Defaulting to \"STANDARD (BRANCH)\".");
		methodName = "BRANCH";
		return ged::Options::GEDMethod::BRANCH;
	}
	if (methodString == "FAST" || methodString == "BRANCH_FAST") {
		methodName = "BRANCH_FAST";
		return ged::Options::GEDMethod::BRANCH_FAST;
	}
	else if (methodString == "STANDARD" || methodString == "BRANCH") {
		methodName = "BRANCH";
		return ged::Options::GEDMethod::BRANCH;
	}
	else if (methodString == "TIGHT" || methodString == "BRANCH_TIGHT") {
		methodName = "BRANCH_TIGHT";
		return ged::Options::GEDMethod::BRANCH_TIGHT;}
	else
		throwError("Couldn't construct HGC Environment:", "\"" + methodString + "\" is an invalid GED method.");
}

// checks if the given graph id belongs to a graph that was generated using a sample in the csv format
bool HGCGED::isSampleGraph(ged::GEDGraph::GraphID graphId) {
	return graphId < sampleNamesToFeatures.size();
}

#pragma endregion

#pragma region setup

// constructs the HGCGEDExec environment
HGCGED::HGCGED(const std::string& methodString, const std::string& methodArguments, bool useCustomEditCosts, const std::string& initTypeString):
	customEditCosts{nullptr},
	datasetEditCosts{nullptr} {

	// ged env setup
	ged_ = new ged::GEDEnv<std::size_t, std::size_t, double>;

	// edit costs
	if (useCustomEditCosts) {
		customEditCosts = new UserDefined<std::size_t, double>();
		ged_->set_edit_costs(customEditCosts);
		editCostsName = "custom";
	}
	else {
		ged_->set_edit_costs(ged::Options::EditCosts::CONSTANT);
		editCostsName = "constant";
	}

	// init type
	ged_->init(parseInitType(initTypeString));

	// method
	ged::Options::GEDMethod method = loadMethod(methodString);
	ged_->set_method(method, methodArguments);
	ged_->init_method();
	if (customEditCosts)
		customEditCosts->multiThreaded = (parseMethodThread(methodArguments) > 1);

}

// destructs HGCGEDExec environment
HGCGED::~HGCGED() {
	delete ged_;
	delete customEditCosts;
	delete datasetEditCosts;
}

#pragma endregion

#pragma region csv

// gets an omics dataset with an optional costs dataset, parses them and creates ged graphs and edit costs out of them
void HGCGED::loadOmicsData(const std::string& omicsDatasetPath, const std::string& associatedCostsDatasetPath = "", char separator = ',') {

	auto* ged = new ged::GEDEnv<std::size_t, std::size_t, double>();

	#pragma region parse omics dataset

	dicoda::CSVParser csvParser;
	csvParser.parse(omicsDatasetPath, separator);

	// check for duplicate sample names
	std::unordered_set<std::string> sampleNamesSet;
	for (std::size_t rowIndex = 1; rowIndex < csvParser.num_rows(); rowIndex++) {
		sampleNamesSet.emplace(csvParser.cell(rowIndex, 0));
	}
	if (sampleNamesSet.size() != csvParser.num_rows() - 1) {
		throwError("Couldn't load omics data:", "Duplicate sample names in \"" + omicsDatasetPath + "\".");
	}

	// check for duplicate feature names
	std::unordered_set<std::string> featureNamesSet;
	for (std::size_t columnIndex = 1; columnIndex < csvParser.num_columns(); columnIndex++) {
		featureNamesSet.emplace(csvParser.cell(0, columnIndex));
	}
	if (featureNamesSet.size() != csvParser.num_columns() - 1) {
		throwError("Couldn't load omics data:", "Duplicate feature names in \"" + omicsDatasetPath + "\".");
	}

	#pragma endregion

	#pragma region add new omics data to existing omics data

	std::size_t firstNonSampleGraphId = sampleNamesToFeatures.size();

	// ensuring the exact same features names
	if (!sampleNamesToFeatures.empty()) {
		std::map<std::string, double> featureNames = sampleNamesToFeatures.begin()->second;
		for (std::size_t columnIndex = 1; columnIndex < csvParser.num_columns(); columnIndex++) {
			const std::string& featureName = csvParser.cell(0, columnIndex);
			if (featureNames.find(featureName) == featureNames.end()) {
				throwError("Couldn't load omics data:", "When you already loaded omics data before, additional omics data must contain the exact same feature names as the first, but \"" + featureName + "\" is a new feature name.");
			}
		}
	}

	// add the samples to the current omics data
	for (std::size_t rowIndex = 1; rowIndex < csvParser.num_rows(); rowIndex++) {
		const std::string& sampleName = csvParser.cell(rowIndex, 0);
		if (sampleNamesToFeatures.find(sampleName) != sampleNamesToFeatures.end()) {
			showWarning("HGC Environment already contains a sample with name \"" + sampleName + "\"! It will be overwritten.");
			sampleNamesToFeatures.erase(sampleName);
		}

		std::map<std::string, double> featureNamesToFeatureValues;
		for (std::size_t columnIndex = 1; columnIndex < csvParser.num_columns(); columnIndex++) {
			const std::string& featureName = csvParser.cell(0, columnIndex);
			double featureValue;
			try {
				featureValue = std::stod(csvParser.cell(rowIndex, columnIndex));
			}
			catch (std::invalid_argument& e) {
				throwError("Couldn't load omics data:", "Feature \"" + featureName + "\" of sample \"" + sampleName + "\" has a non numeric value."); // NOLINT(performance-inefficient-string-concatenation)
			}
			if (featureValue < 0) {
				throwError("Couldn't load omics data:", "Feature \"" + featureName + "\" of sample \"" + sampleName + "\" has a negative value."); // NOLINT(performance-inefficient-string-concatenation)
			}
			featureNamesToFeatureValues.emplace(featureName, featureValue);
		}
		sampleNamesToFeatures.emplace(sampleName, featureNamesToFeatureValues);
	}

	#pragma endregion

	#pragma region reconstruct sample graphs into new ged environment

	#pragma region setup id-based sample & feature storage

	std::vector<std::string> sampleNames;
	std::vector<std::string> featureNames;
	ged::DMatrix omicsDataMatrix = ged::DMatrix(sampleNamesToFeatures.size(), sampleNamesToFeatures.begin()->second.size());
	ged::Matrix<bool> isNodeMatrix = ged::Matrix<bool>(sampleNamesToFeatures.size(), sampleNamesToFeatures.begin()->second.size());

	std::size_t sampleId = 0;
	auto sampleIterator = sampleNamesToFeatures.begin();
	while (sampleIterator != sampleNamesToFeatures.end()) {
		std::string sampleName = sampleIterator->first;
		std::map<std::string, double> featureNamesToFeatureValues = sampleIterator->second;

		sampleNames.emplace_back(sampleName);

		std::size_t featureId = 0;
		auto featureIterator = featureNamesToFeatureValues.begin();
		while (featureIterator != featureNamesToFeatureValues.end()) {
			std::string featureName = featureIterator->first;
			double featureValue = featureIterator->second;

			if (sampleIterator == sampleNamesToFeatures.begin()) {
				featureNames.emplace_back(featureName);
			}
			omicsDataMatrix(sampleId, featureId) = featureValue;
			isNodeMatrix(sampleId, featureId) = featureValue > 0;

			featureId++;
			featureIterator++;
		}
		sampleId++;
		sampleIterator++;
	}

	#pragma endregion

	#pragma region compute logratio aggregates

	double min_logratio_{std::numeric_limits<double>::max()};
	double max_logratio_{std::numeric_limits<double>::min()};
	double max_feature_{std::numeric_limits<double>::min()};

	ged::DMatrix normalized_logratio_means_ = ged::DMatrix(featureNames.size(), featureNames.size());
	ged::DMatrix normalized_logratio_stdevs_ = ged::DMatrix(featureNames.size(), featureNames.size());
	normalized_logratio_means_.set_to_val(dicoda::undefined_double());
	normalized_logratio_stdevs_.set_to_val(dicoda::undefined_double());

	// get min_logratio, max_logratio & max_featuere
	for (std::size_t sample_id{0}; sample_id < sampleNames.size(); sample_id++) {

		for (std::size_t feature_id_1{0}; feature_id_1 < featureNames.size() - 1; feature_id_1++) {

			max_feature_ = std::max(max_feature_, omicsDataMatrix(sample_id, feature_id_1));
			if (not isNodeMatrix(sample_id, feature_id_1)) {
				continue;
			}
			for (std::size_t feature_id_2{feature_id_1 + 1}; feature_id_2 < featureNames.size(); feature_id_2++) {
				if (not isNodeMatrix(sample_id, feature_id_2)) {
					continue;
				}
				double featureValue1 = omicsDataMatrix(sample_id, feature_id_1);
				double featureValue2 = omicsDataMatrix(sample_id, feature_id_2);
				double logratio{std::log(featureValue1 / featureValue2)};
				min_logratio_ = std::min(min_logratio_, logratio);
				max_logratio_ = std::max(max_logratio_, logratio);
			}
		}
	}

	// get normalized_logratio_means_ & normalized_logratio_stdevs_
	for (std::size_t feature_id_1{0}; feature_id_1 < featureNames.size() - 1; feature_id_1++) {
		for (std::size_t feature_id_2{feature_id_1 + 1}; feature_id_2 < featureNames.size(); feature_id_2++) {
			std::vector<double> normalized_logratios;
			double mean{0};
			for (std::size_t sample_id{0}; sample_id < sampleNames.size(); sample_id++) {
				if (isNodeMatrix(sample_id, feature_id_1) and isNodeMatrix(sample_id, feature_id_2)) {
					double logratio{std::log(omicsDataMatrix(sample_id, feature_id_1) / omicsDataMatrix(sample_id, feature_id_2))};
					normalized_logratios.emplace_back((logratio - min_logratio_) / (max_logratio_ - min_logratio_));
					mean += normalized_logratios.back();
				}
			}
			mean /= static_cast<double>(normalized_logratios.size());
			if (!normalized_logratios.empty()) {
				normalized_logratio_means_(feature_id_1, feature_id_2) = mean;
				double stdev{0};
				for (auto normalized_logratio : normalized_logratios) {
					stdev += (mean - normalized_logratio) * (mean - normalized_logratio);
				}
				stdev /= static_cast<double>(normalized_logratios.size());
				stdev = std::sqrt(stdev);
				normalized_logratio_stdevs_(feature_id_1, feature_id_2) = stdev;
			}
		}
	}

	#pragma endregion

	#pragma region construct bins

	double abundance_threshold_{0.0};
	int number_bins_{100};

	std::vector<std::vector<dicoda::Bin>> sample_bins_;
	double bin_size = max_feature_ / number_bins_;

	for (std::size_t sample_id{0}; sample_id < sampleNames.size(); sample_id++) {
		double upper_bound_last = 0.0;
		std::vector<dicoda::Bin> vec_bin;
		for (int i = 0; i < number_bins_; i++) {
			dicoda::Bin bin;
			bin.lower_bound = upper_bound_last;
			bin.upper_bound = upper_bound_last+bin_size;
			bin.number = i;
			upper_bound_last = bin.upper_bound;
			vec_bin.emplace_back(bin);
		}

		for (std::size_t feature_id{0}; feature_id < featureNames.size(); feature_id++) {
			double value = omicsDataMatrix(sample_id,feature_id);
			if (value == 0 or value < abundance_threshold_) {
				continue;
			}
			auto it_vec_bin = vec_bin.begin();
			while (it_vec_bin != vec_bin.end()) {

				bool insert = it_vec_bin->check_add_feature(feature_id,value);
				if (insert) {
					break;
				}

				++it_vec_bin;
			}
		}
		auto it_vec_bin = vec_bin.begin();

		while (it_vec_bin != vec_bin.end()) {
			it_vec_bin->compute_mean();
			++it_vec_bin;
		}
		sample_bins_.emplace_back(vec_bin);
	}

	#pragma endregion

	#pragma region construct sample bin pairs

	ged::Matrix<std::size_t> num_samples_with_bin_pair_ = ged::Matrix<std::size_t>(number_bins_,number_bins_);
	num_samples_with_bin_pair_.set_to_val(0);

	auto it_sample_bin = sample_bins_.begin();
	while (it_sample_bin != sample_bins_.end()) {
		std::vector<dicoda::Bin> bins_current_sample = *it_sample_bin;

		for (auto it_bin_current_sample_1 = bins_current_sample.begin() ; it_bin_current_sample_1 != bins_current_sample.end(); ++it_bin_current_sample_1) {
			for (auto& it_bin_current_sample_2 : bins_current_sample) {
				if (it_bin_current_sample_1->number == it_bin_current_sample_2.number) {
					continue;
				}

				if (it_bin_current_sample_1->has_features and it_bin_current_sample_2.has_features) {
					int i = it_bin_current_sample_1->number;
					int j = it_bin_current_sample_2.number;

					int x = (int) num_samples_with_bin_pair_(i, j);
					x++;
					num_samples_with_bin_pair_(i, j) = x;
				}
			}
		}
		++it_sample_bin;
	}

	#pragma endregion

	#pragma region generate graphs

	std::size_t min_cutoff_size_{10};
	double z_score_cutoff_{2};

	for (std::size_t sample_id{0}; sample_id < sampleNames.size(); sample_id++) {
		ged::GEDGraph::GraphID graph_id{ged->add_graph(sampleNames.at(sample_id))};
		std::map<std::size_t, std::size_t> feature_ids_to_node_ids;
		std::size_t node_id{0};

		for (std::size_t feature_id{0}; feature_id < featureNames.size(); feature_id++) {
			if (isNodeMatrix(sample_id, feature_id)) {
				feature_ids_to_node_ids.emplace(feature_id, node_id);
				ged->add_node(graph_id, node_id++, feature_id);
			}
		}

		const std::vector<dicoda::Bin>& bins_current_sample = sample_bins_.at(sample_id);

		for (auto it_bin_current_sample_1 = bins_current_sample.begin(); it_bin_current_sample_1 != bins_current_sample.end(); ++it_bin_current_sample_1) {
			if (!it_bin_current_sample_1->has_features) {
				continue;
			}
			for (const auto& it_bin_current_sample_2 : bins_current_sample) {
				if (it_bin_current_sample_1->number == it_bin_current_sample_2.number) {
					continue;
				}
				if(!it_bin_current_sample_2.has_features) {
					continue;
				}

				double logratio{std::log(it_bin_current_sample_1->mean_value / it_bin_current_sample_2.mean_value)};
				double normalized_logratio{(logratio - min_logratio_) / (max_logratio_ - min_logratio_)};

				bool add_edge{true};

				if (num_samples_with_bin_pair_(it_bin_current_sample_1->number,it_bin_current_sample_2.number)>=min_cutoff_size_) {
					double z_score{(normalized_logratio - normalized_logratio_means_(static_cast<std::size_t>(it_bin_current_sample_1->mean_value), static_cast<std::size_t>(it_bin_current_sample_2.mean_value))) / normalized_logratio_stdevs_(static_cast<std::size_t>(it_bin_current_sample_1->mean_value), static_cast<std::size_t>(it_bin_current_sample_2.mean_value))};
					if (std::fabs(z_score) < z_score_cutoff_) {
						add_edge = false;
					}
				}

				if (add_edge) {

					int k = 0;
					for (auto it_bin1_features = it_bin_current_sample_1->feature_ids.begin(); it_bin1_features != it_bin_current_sample_1->feature_ids.end(); ++it_bin1_features) {
						int l = 0;
						double value_1 = it_bin_current_sample_1->values.at(k);

						for (auto it_bin2_features = it_bin_current_sample_2.feature_ids.begin(); it_bin2_features != it_bin_current_sample_2.feature_ids.end(); ++it_bin2_features) {
							double value_2 = it_bin_current_sample_2.values.at(l);
							double logratio_exact{std::log(value_1 / value_2)};
							double normalized_logratio_exact{(logratio_exact - min_logratio_) / (max_logratio_ - min_logratio_)};

							ged->add_edge(graph_id, feature_ids_to_node_ids.at(*it_bin1_features), feature_ids_to_node_ids.at(*it_bin2_features), normalized_logratio_exact);
							l++;
						}
						k++;
					}
				}
			}
		}
	}

	#pragma endregion

	#pragma endregion

	#pragma region copy non sample graphs into new ged environment

	// copy non-sample graphs from old ged into new ged
	for (std::size_t nonSampleGraphId = firstNonSampleGraphId; nonSampleGraphId < ged_->num_graphs(); nonSampleGraphId++) {
		std::string nonSampleGraphName = ged_->get_graph_name(nonSampleGraphId);

		// if attributes are loaded, check if the graph name is already used
		if (!sampleNamesToAttributes.empty()) {
			for (std::size_t sampleGraphId = 0; sampleGraphId < ged->num_graphs(); sampleGraphId++) {
				const std::string& sampleGraphName = ged->get_graph_name(sampleGraphId);
				if (nonSampleGraphName == sampleGraphName) {
					showWarning("Adding the sample with name \"" + sampleGraphName + "\" will result in the HGC Environment containing mutiple graphs with that name afterwards. This might lead to unwanted behavior when associating attribute data to graphs.");
				}
			}
		}

		// copy graph
		ged::ExchangeGraph<std::size_t, std::size_t, double> graph = ged_->get_graph(nonSampleGraphId, false, false, true);
		ged::GEDGraph::GraphID graphId = ged->add_graph(nonSampleGraphName);
		for (std::size_t nodeId : graph.original_node_ids) {
			ged->add_node(graphId, nodeId, graph.node_labels.at(nodeId));
		}
		for (std::pair<std::pair<std::size_t, std::size_t>, double> edge : graph.edge_list) {
			ged->add_edge(graphId, edge.first.first, edge.first.second, edge.second);
		}
	}

	#pragma endregion

	#pragma region deal with edit costs

	if (!associatedCostsDatasetPath.empty()) {
		delete(datasetEditCosts);

		// parse & check the data
		csvParser.parse(associatedCostsDatasetPath, separator);
		if (csvParser.num_rows() != csvParser.num_columns()) {
			throwError("Couldn't load costs data:", "\"" + associatedCostsDatasetPath + "\" has an unequal number of rows and columns.");
		}

		// build costs matrix
		std::map<std::string, std::size_t> featureNamesToFeatureIds;
		for (std::size_t i = 0; i < featureNames.size(); i++) {
			featureNamesToFeatureIds.emplace(featureNames.at(i), i);
		}

		ged::DMatrix nodeRelabelingCosts = ged::DMatrix(featureNames.size(), featureNames.size(), -1.f);

		for (std::size_t rowIndex = 1; rowIndex < csvParser.num_rows(); rowIndex++) {
			const std::string& featureNameRowIndex = csvParser.cell(rowIndex, 0);
			std::size_t featureIdRowIndex;
			try {
				featureIdRowIndex = featureNamesToFeatureIds.at(featureNameRowIndex);
			}
			catch (std::out_of_range& error) {
				showWarning("Feature \"" + featureNameRowIndex + "\" is not part of the omics data. Its costs data will be ignored.");
				continue;
			}

			for (std::size_t columnIndex = 1; columnIndex < csvParser.num_columns(); columnIndex++) {
				const std::string& featureNameColumnIndex = csvParser.cell(0, columnIndex);
				std::size_t featureIdColumnIndex;
				try {
					featureIdColumnIndex = featureNamesToFeatureIds.at(featureNameColumnIndex);
				}
				catch (std::out_of_range& error) {
					continue;   // showing a warning here would be redundant
				}

				double featureCost;
				try {
					featureCost = std::stod(csvParser.cell(rowIndex, columnIndex));
				}
				catch (std::invalid_argument& e) {
					throwError("Couldn't load costs data:", "Cost between features \"" + featureNameRowIndex + "\" and \"" + featureNameColumnIndex + "\" has non numeric value."); // NOLINT(performance-inefficient-string-concatenation)
				}
				nodeRelabelingCosts(featureIdRowIndex, featureIdColumnIndex) = featureCost;
			}
		}

		for (std::size_t rowIndex = 0; rowIndex < nodeRelabelingCosts.num_rows(); rowIndex++) {
			if (nodeRelabelingCosts(rowIndex, 0) < 0) {
				showWarning("Costs dataset is missing costs data for feature \"" + featureNames.at(rowIndex) + "\". Defaulting to costs of 1. This might lead to inconsistent results.");
				for (std::size_t columnIndex = 0; columnIndex < nodeRelabelingCosts.num_cols(); columnIndex++) {
					nodeRelabelingCosts(rowIndex, columnIndex) = 1.f;
					nodeRelabelingCosts(columnIndex, rowIndex) = 1.f;
				}
				nodeRelabelingCosts(rowIndex, rowIndex) = 0.f;
			}
		}

		nodeRelabelingCosts /= nodeRelabelingCosts.max();

		// set edit costs
		editCostsName = "dataset";

		datasetEditCosts = new HGCCosts<std::size_t, double>(nodeRelabelingCosts);
	}
	else {
		if (customEditCosts) {
			editCostsName = "custom";
		}
		else {
			editCostsName = "constant";
		}
	}

	#pragma endregion

	#pragma region initialize new ged environment

	if (editCostsName == "dataset") {
		ged->set_edit_costs(datasetEditCosts);
	}
	else if (editCostsName == "custom") {
		ged->set_edit_costs(customEditCosts);
	}
	else {  // if constant
		ged->set_edit_costs(ged::Options::EditCosts::CONSTANT);
	}
	ged->set_method(loadMethod(methodName));
	ged->init(ged_->get_init_type());
	ged->init_method();

	delete(ged_);
	ged_ = ged;

	#pragma endregion

}

// gets a clinical attribute dataset and parses it
void HGCGED::loadAttributesData(const std::string& attributesDatasetPath, char separator = ',') {

	// parse dataset
	dicoda::CSVParser csvParser;
	csvParser.parse(attributesDatasetPath, separator);

	// check for duplicate sample names
	std::unordered_set<std::string> sampleNames;
	for (std::size_t rowIndex = 1; rowIndex < csvParser.num_rows(); rowIndex++) {
		sampleNames.emplace(csvParser.cell(rowIndex, 0));
	}
	if (sampleNames.size() != csvParser.num_rows() - 1) {
		throwError("Couldn't load attributes data:", "Duplicate sample names in \"" + attributesDatasetPath + "\".");
	}

	// add the attributes to the current attributes data
	for (std::size_t rowIndex = 1; rowIndex < csvParser.num_rows(); rowIndex++) {
		const std::string& sampleName = csvParser.cell(rowIndex, 0);
		if (sampleNamesToAttributes.find(sampleName) != sampleNamesToAttributes.end()) {
			showWarning("HGC Environment already contains attributes data for a sample with name \"" + sampleName + "\". They will be overwritten.");
			sampleNamesToAttributes.erase(sampleName);
		}

		std::map<std::string, std::string> attributeNamesToAttributeValues;
		for (std::size_t columnIndex = 1; columnIndex < csvParser.num_columns(); columnIndex++) {
			const std::string& attributeName = csvParser.cell(0, columnIndex);
			const std::string& attributeValue = csvParser.cell(rowIndex, columnIndex);
			attributeNamesToAttributeValues.emplace(attributeName, attributeValue);
		}
		sampleNamesToAttributes.emplace(sampleName, attributeNamesToAttributeValues);
	}

}

#pragma endregion

#pragma region hgc

// generates a vector containing labels for the graphs
void HGCGED::generateLabels(const std::string& labeledAttribute = "") {
	if (!ged_)
		throwError("Couldn't generate graph labels:", "HGC environment not constructed.");

	// reset vector
	labelVector = std::vector<std::string>(ged_->num_graphs(), "");

	// generate name vector if no attribute is passed
	if (labeledAttribute.empty()) {
		for (std::size_t graphId = 0; graphId < ged_->num_graphs(); graphId++)
			labelVector.at(graphId) = std::to_string(graphId) + "_" + ged_->get_graph_name(graphId);
	}
	else {
		for (std::size_t graphId = 0; graphId < ged_->num_graphs(); graphId++) {
			std::string graphName = ged_->get_graph_name(graphId);

			// use name as label if the environment doesn't contain attributes for this graph
			if (sampleNamesToAttributes.find(graphName) == sampleNamesToAttributes.end()) {
				if (isSampleGraph(graphId)) {
					showWarning("The graph of sample \"" + graphName + "\" has no associated attributes. Using it's name as it's label instead.");
				}
				labelVector.at(graphId) = std::to_string(graphId) + "_" + graphName;
			}
			else {
				std::map<std::string, std::string> attributeNamesToAttributeValues = sampleNamesToAttributes.at(graphName);

				// use name as label if the graph doesn't contain the graph attribute
				if (attributeNamesToAttributeValues.find(labeledAttribute) == attributeNamesToAttributeValues.end()) {
					showWarning("The attributes of graph \"" + graphName + "\" do not contain \"" + labeledAttribute + "\". Using it's name as it's label instead."); // NOLINT(performance-inefficient-string-concatenation)
					labelVector.at(graphId) = std::to_string(graphId) + "_" + graphName;
				}
					// use attribute as label
				else {
					labelVector.at(graphId) = std::to_string(graphId) + "_" + attributeNamesToAttributeValues.at(labeledAttribute);
				}
			}
		}
	}

}

// the actual implementation of the method that computes the ged matrix
void HGCGED::computeGedsGilScope() {

	// security
	if (!ged_)
		throwError("Couldn't compute graph edit distances:", "HGC environment not constructed.");

	// setup results
	distanceMatrix = std::vector<std::vector<int>>(ged_->num_graphs(), std::vector<int>(ged_->num_graphs(), -1));

	// setup console output
	std::cout << std::fixed << std::setprecision(2) << std::endl;
	float scaler;
	if (ged_->num_graphs() == 0) {
		std::cout << "\033[A\33[KEnvironment is empty. Nothing to compute." << std::endl;
		return;
	}
	else
		scaler = 100.0f / static_cast<float>(ged_->num_graphs() * ged_->num_graphs());

	// run method & save results
	std::size_t c = 0;
	for (std::size_t graphId1 = 0; graphId1 < ged_->num_graphs(); graphId1++) {

		for (std::size_t graphId2 = 0; graphId2 < ged_->num_graphs(); graphId2++) {

			if (graphId1 == graphId2)
				distanceMatrix.at(graphId1).at(graphId2) = 0;
			else {
				ged_->run_method(graphId1, graphId2);
				distanceMatrix.at(graphId1).at(graphId2) = static_cast<int>(ged_->get_upper_bound(graphId1, graphId2));
			}

			std::cout << "\033[A\33[KProgress: " + std::to_string(static_cast<float>(c+1) * scaler) + "%" << std::endl;
			c++;	// pun intended
		}

	}

	if (methodName == "IPFP") { // Makes the distance matrix symmetrical. This should only be nessecary when using a ranomized ged method (which is currently not supported by hgc).
		showInfo("Processing results...");

		for (std::size_t i = 0; i < distanceMatrix.size(); i++) {
			for (std::size_t j = i; j < distanceMatrix.at(i).size(); j++) {
				int first = distanceMatrix.at(i).at(j);
				int second = distanceMatrix.at(j).at(i);
				if (first < second)
					distanceMatrix.at(j).at(i) = first;
				else if (first > second)
					distanceMatrix.at(i).at(j) = second;
			}
		}
	}

}

// a helper function that calls a function which contains the actual implementation within a scope in which the GIL is and stays aquired if needed
void HGCGED::computeGeds() {
	if (customEditCosts) {
		showWarning("Using custom edit costs significantly decreases performance, especially when used with multi-threading!");
		if (!customEditCosts->multiThreaded) {
			pybind11::gil_scoped_acquire acquire;
			computeGedsGilScope();
		}
		else
			computeGedsGilScope();
	}
	else
		computeGedsGilScope();
}

#pragma endregion

#pragma region pybind11

#pragma region get

// returns the computed GED matrix
std::vector<std::vector<int>> HGCGED::getDistanceMatrix() {
	return distanceMatrix;
}

// returns the label vector
std::vector<std::string> HGCGED::getLabelVector() {
	return labelVector;
}

// return a string of the method in use
std::string HGCGED::getMethodName() {
	return methodName;
}

// returns a string of the edit costs in use
std::string HGCGED::getEditCostsName() {
	return editCostsName;
}

// returns the graph with the given id in the ged environment as a triple containing its adjacency matrix, its node labels and its edge labels
std::vector<std::variant<std::vector<std::vector<std::size_t>>, std::vector<std::size_t>, std::map<std::pair<std::size_t, std::size_t>, double>>> HGCGED::getGraph(ged::GEDGraph::GraphID id) {

	if (!ged_)
		throwError("Couldn't get graph:", "HGC environment not constructed.");

	if (id >= ged_->num_graphs())
		throwError("Couldn't get graph:", "A graph with ID " + std::to_string(id) + " is not contained in the environment.");

	ged::ExchangeGraph<std::size_t, std::size_t, double> graph = ged_->get_graph(id, true, false, false);

	std::vector<std::variant<
		std::vector<std::vector<std::size_t>>,					// adjacency matrix type
		std::vector<std::size_t>,								// node label type
		std::map<std::pair<std::size_t, std::size_t>, double>	// edge label type
	>> graph_data = std::vector<std::variant<std::vector<std::vector<std::size_t>>, std::vector<std::size_t>, std::map<std::pair<std::size_t, std::size_t>, double>>>();
	graph_data.emplace_back(graph.adj_matrix);
	graph_data.emplace_back(graph.node_labels);
	graph_data.emplace_back(graph.edge_labels);

	return graph_data;
}

// returns the name of the graph with the given id in the ged environment
std::string HGCGED::getGraphName(ged::GEDGraph::GraphID id) {
	if (!ged_)
		throwError("Couldn't get graph name:", "HGC environment not constructed.");
	if (id >= ged_->num_graphs())
		throwError("Couldn't get graph name:", "A graph with ID " + std::to_string(id) + " is not contained in the environment.");

	return ged_->get_graph_name(id);
}

// returns the number of graphs contained in the ged environment
std::size_t HGCGED::getNumberOfGraphs() {
	if (!ged_)
		throwError("Couldn't get number of graphs:", "HGC environment not constructed.");

	return ged_->num_graphs();
}

#pragma endregion

#pragma region set

// adds an empty graph into the ged environment
std::size_t HGCGED::addGraph(const std::string& graphName) {
	if (!ged_)
		throwError("Couldn't add graph:", "HGC environment not constructed.");

	// if attributes are loaded, check if the graph name is already used
	if (!sampleNamesToAttributes.empty()) {
		for (std::size_t graphId = 0; graphId < ged_->num_graphs(); graphId++) {
			if (ged_->get_graph_name(graphId) == graphName) {
				showWarning("HGC Environment already contains a graph with name \"" + graphName + "\". This might lead to unwanted behavior when associating attribute data to graphs.");
			}
		}
	}

	ged::GEDGraph::GraphID id = ged_->add_graph(graphName, "");
	return id;
}

// adds a node to the graph with the given id in the ged environment
void HGCGED::addNode(ged::GEDGraph::GraphID graphID, std::size_t nodeID, std::size_t nodeLabel) {
	if (!ged_)
		throwError("Couldn't add node:", "HGC environment not constructed.");

	ged_->add_node(graphID, nodeID, nodeLabel);
}

// adds an edge to the graph with the given id in the ged environment
void HGCGED::addEdge(ged::GEDGraph::GraphID graphID, std::size_t nodeIDFrom, std::size_t nodeIDTo, double edgeLabel) {
	if (!ged_)
		throwError("Couldn't add edge:", "HGC environment not constructed.");

	ged_->add_edge(graphID, nodeIDFrom, nodeIDTo, edgeLabel, true);
}

// reinitializes the ged environment
void HGCGED::reinitGed() {
	if (!ged_)
		throwError("Couldn't reinitialize environment:", "HGC environment not constructed.");

	ged_->init(ged_->get_init_type());
	ged_->init_method();
}

#pragma endregion

#pragma endregion

#pragma region Tests

// a playgorund for tests
[[maybe_unused]] void HGCGED::runTests() {
	debugLog("Running tests...");

	/*std::string omicsDataset = "../src/temp/data/test_otu_abundances_small.csv";
	std::string costsDataset = "../src/temp/data/test_otu_distances_small.csv";
	std::string attributesDataset = "../src/temp/data/test_clinical_data.csv";

	loadOmicsData(omicsDataset, costsDataset);

	std::string graphName1 = "ManualGraph1";
	addGraph(graphName1);
	std::string graphName2 = "ManualGraph2";
	addGraph(graphName2);

	loadOmicsData(omicsDataset);

	std::string graphName3 = "ManualGraph3";
	addGraph(graphName3);
	std::string graphName4 = "ManualGraph4";
	addGraph(graphName4);

	std::string omicsDataset2 = "../src/temp/data/test_otu_abundances_smaller.csv";
	loadOmicsData(omicsDataset2, costsDataset);

	loadAttributesData(attributesDataset);
	std::string graphNameWarning = "01_TM48_FDA";
	addGraph(graphNameWarning);

	generateLabels();
	std::string labelName = "HSCT_responder";
	generateLabels(labelName);*/

	reinitGed();
	computeGeds();

	debugLog("breakpoint");
}

#pragma endregion

PYBIND11_MODULE(HGCGED, module) {

	pybind11::class_<HGCGED>(module, "HGCGED")
			// setup
			.def(pybind11::init<std::string&, std::string&, bool, std::string&>())
			// csv
			.def("load_omics_data", &HGCGED::loadOmicsData)
			.def("load_attributes_data", &HGCGED::loadAttributesData)
			// run
			.def("generate_labels", &HGCGED::generateLabels)
			.def("compute_geds", &HGCGED::computeGeds, pybind11::call_guard<pybind11::gil_scoped_release>())
			// set
			.def("add_graph", &HGCGED::addGraph)
			.def("add_node", &HGCGED::addNode)
			.def("add_edge", &HGCGED::addEdge)
			.def("reinit_ged", &HGCGED::reinitGed)
			// get
			.def("get_number_of_graphs", &HGCGED::getNumberOfGraphs)
			.def("get_graph_name", &HGCGED::getGraphName)
			.def("get_graph", &HGCGED::getGraph)
			.def("get_method_name", &HGCGED::getMethodName)
			.def("get_edit_costs_name", &HGCGED::getEditCostsName)
			.def("get_label_vector", &HGCGED::getLabelVector)
			.def("get_distance_matrix", &HGCGED::getDistanceMatrix)
			// other
			.def("run_tests_external", &HGCGED::runTests, pybind11::call_guard<pybind11::gil_scoped_release>());

}
