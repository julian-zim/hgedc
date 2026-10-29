#ifndef HGCCPP_BIN_HPP
#define HGCCPP_BIN_HPP

#include <cstddef>
#include <vector>

// a bin of feature values within a fixed abundance range, used to discretize omics data into graph edges
struct Bin {
	int number;
	std::vector<std::size_t> feature_ids;
	std::vector<double> values;
	double mean_value;
	double lower_bound;
	double upper_bound;
	bool has_features{false};

	bool check_add_feature(std::size_t feature_id, double value) {
		if (value >= lower_bound and value < upper_bound) {
			feature_ids.emplace_back(feature_id);
			values.emplace_back(value);
			has_features = true;
			return true;
		}
		return false;
	}

	void compute_mean() {
		mean_value = 0.0;
		for (const auto& value : values) {
			mean_value += value;
		}
		if (!values.empty()) {
			mean_value /= static_cast<double>(values.size());
		}
	}
};

#endif //HGCCPP_BIN_HPP
