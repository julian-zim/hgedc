#ifndef SRC_HGC_COSTS_HPP_
#define SRC_HGC_COSTS_HPP_

template<class UserNodeLabel, class UserEdgeLabel>
class HGCCosts : public ged::EditCosts<UserNodeLabel, UserEdgeLabel> {

public:
	explicit HGCCosts(const ged::DMatrix& node_rel_costs, double node_factor = 0.5, double ins_del_factor = 0.5);
	virtual ~HGCCosts();

	virtual double node_ins_cost_fun(const UserNodeLabel& node_label) const final;
	virtual double node_del_cost_fun(const UserNodeLabel& node_label) const final;
	virtual double node_rel_cost_fun(const UserNodeLabel& node_label_1, const UserNodeLabel& node_label_2) const final;
	virtual double edge_ins_cost_fun(const UserEdgeLabel& edge_label) const final;
	virtual double edge_del_cost_fun(const UserEdgeLabel& edge_label) const final;
	virtual double edge_rel_cost_fun(const UserEdgeLabel& edge_label_1, const UserEdgeLabel& edge_label_2) const final;

private:
	ged::DMatrix nodeRelabelCosts;
	double nodeFactor;
	double insertDeleteFactor;

};

#ifndef SRC_HGC_COSTS_IPP_
#define SRC_HGC_COSTS_IPP_

template<class UserNodeLabel, class UserEdgeLabel>
HGCCosts<UserNodeLabel, UserEdgeLabel>::
HGCCosts(const ged::DMatrix& node_rel_costs, double node_factor, double ins_del_factor) {
	nodeRelabelCosts = node_rel_costs;
	nodeFactor = node_factor;
	insertDeleteFactor = ins_del_factor;
}

template<class UserNodeLabel, class UserEdgeLabel>
HGCCosts<UserNodeLabel, UserEdgeLabel>::
~HGCCosts() = default;

template<class UserNodeLabel, class UserEdgeLabel>
double
HGCCosts<UserNodeLabel, UserEdgeLabel>::
node_ins_cost_fun(const UserNodeLabel& node_label) const {
	return insertDeleteFactor * nodeFactor;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
HGCCosts<UserNodeLabel, UserEdgeLabel>::
node_del_cost_fun(const UserNodeLabel& node_label) const {
	return insertDeleteFactor * nodeFactor;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
HGCCosts<UserNodeLabel, UserEdgeLabel>::
node_rel_cost_fun(const UserNodeLabel& node_label_1, const UserNodeLabel& node_label_2) const {
	double result = (1 - insertDeleteFactor) * nodeFactor;
	if (node_label_1 >= nodeRelabelCosts.num_rows() || node_label_2 >= nodeRelabelCosts.num_cols()) {
		return result;
	}
	return result * nodeRelabelCosts(node_label_1, node_label_2);
}

template<class UserNodeLabel, class UserEdgeLabel>
double
HGCCosts<UserNodeLabel, UserEdgeLabel>::
edge_ins_cost_fun(const UserEdgeLabel& edge_label) const {
	return insertDeleteFactor * (1 - nodeFactor);
}

template<class UserNodeLabel, class UserEdgeLabel>
double
HGCCosts<UserNodeLabel, UserEdgeLabel>::
edge_del_cost_fun(const UserEdgeLabel& edge_label) const {
	return insertDeleteFactor * (1 - nodeFactor);
}

template<class UserNodeLabel, class UserEdgeLabel>
double
HGCCosts<UserNodeLabel, UserEdgeLabel>::
edge_rel_cost_fun(const UserEdgeLabel& edge_label_1, const UserEdgeLabel& edge_label_2) const {
	return (1 - insertDeleteFactor) * (1 - nodeFactor) * std::fabs(edge_label_1 - edge_label_2);
}

#endif /* SRC_HGC_COSTS_IPP_ */

#endif /* SRC_HGC_COSTS_HPP_ */
