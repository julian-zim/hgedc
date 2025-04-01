#ifndef HGCCPP_HGCGED_H
#define HGCCPP_HGCGED_H

#include "src/env/ged_env.hpp"
#include "include/types.hpp"
#include "include/bin.hpp"
#include "include/csv_parser.hpp"

#include "HGCCosts.hpp"
#include "UserDefined.hpp"

class HGCGED {

private:
	// resources
	ged::GEDEnv<std::size_t, std::size_t, double>* ged_;
	UserDefined<std::size_t, double>* customEditCosts;
	HGCCosts<std::size_t, double>* datasetEditCosts;

	// csv data
	std::map<std::string, std::map<std::string, double>> sampleNamesToFeatures;			// contains the omics data
	std::map<std::string, std::map<std::string, std::string>> sampleNamesToAttributes;	// contains the attributes data

	// results
	std::vector<std::vector<int>> distanceMatrix;
	std::vector<std::string> labelVector;

	// info
	std::string editCostsName;
	std::string methodName;

public:
	ged::Options::GEDMethod loadMethod(const std::string& methodString);
	bool isSampleGraph(ged::GEDGraph::GraphID graphId);

	HGCGED(const std::string& methodString, const std::string& methodArguments, bool useCustomEditCosts, const std::string& initTypeString);
	~HGCGED();

	void loadOmicsData(const std::string& omicsDatasetPath, const std::string& associatedCostsDatasetPath, char separator);
	void loadAttributesData(const std::string& attributesDatasetPath, char separator);
	void generateLabels(const std::string& labeledAttribute);

	void computeGedsGilScope();
	void computeGeds();

	std::vector<std::vector<int>> getDistanceMatrix();
	std::vector<std::string> getLabelVector();
	std::string getMethodName();
	std::string getEditCostsName();
	std::vector<std::variant<std::vector<std::vector<std::size_t>>, std::vector<std::size_t>, std::map<std::pair<std::size_t, std::size_t>, double>>> getGraph(ged::GEDGraph::GraphID id);
	std::string getGraphName(ged::GEDGraph::GraphID id);
	std::size_t getNumberOfGraphs();
	std::size_t addGraph(const std::string& graphName);
	void addNode(ged::GEDGraph::GraphID graphID, std::size_t nodeID, std::size_t nodeLabel);
	void addEdge(ged::GEDGraph::GraphID graphID, std::size_t nodeIDFrom, std::size_t nodeIDTo, double edgeLabel);
	void reinitGed();

	[[maybe_unused]] void runTests();
};

#endif //HGCCPP_HGCGED_H
