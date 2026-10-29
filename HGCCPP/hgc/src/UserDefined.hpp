#ifndef SRC_UD_COSTS_HPP_
#define SRC_UD_COSTS_HPP_

#include <pybind11/embed.h>

template<class UserNodeLabel, class UserEdgeLabel>
class UserDefined : public ged::EditCosts<UserNodeLabel, UserEdgeLabel> {

public:
	UserDefined();
	virtual ~UserDefined();

	virtual double node_ins_cost_fun(const UserNodeLabel& node_label) const final;
	virtual double node_del_cost_fun(const UserNodeLabel& node_label) const final;
	virtual double node_rel_cost_fun(const UserNodeLabel& node_label_1, const UserNodeLabel& node_label_2) const final;
	virtual double edge_ins_cost_fun(const UserEdgeLabel& edge_label) const final;
	virtual double edge_del_cost_fun(const UserEdgeLabel& edge_label) const final;
	virtual double edge_rel_cost_fun(const UserEdgeLabel& edge_label_1, const UserEdgeLabel& edge_label_2) const final;

	bool multiThreaded;

private:
	pybind11::module pythonModule;

};

#ifndef SRC_UD_COSTS_IPP_
#define SRC_UD_COSTS_IPP_

template<class UserNodeLabel, class UserEdgeLabel>
UserDefined<UserNodeLabel, UserEdgeLabel>::
UserDefined():
	multiThreaded{false} {
	pythonModule = pybind11::module::import("edit_costs");	// ModuleNotFoundError handled in main python program
}

template<class UserNodeLabel, class UserEdgeLabel>
UserDefined<UserNodeLabel, UserEdgeLabel>::
~UserDefined() = default;

template<class UserNodeLabel, class UserEdgeLabel>
double
UserDefined<UserNodeLabel, UserEdgeLabel>::
node_ins_cost_fun(const UserNodeLabel& node_label) const {
	//std::cout << "Called node_ins_cost_fun constructor" << std::endl;

	double result;
	if (multiThreaded) {
		pybind11::gil_scoped_acquire acquire;
		result = pythonModule.attr("node_ins_cost_fun")(node_label).template cast<double>();
		//result = pybind11::eval("edit_costs.node_ins_cost_fun(" + std::to_string(node_label) + ")").template cast<double>();
	}
	else {
		result = pythonModule.attr("node_ins_cost_fun")(node_label).template cast<double>();
		//result = pybind11::eval("edit_costs.node_ins_cost_fun(" + std::to_string(node_label) + ")").template cast<double>();
	}
	return result;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
UserDefined<UserNodeLabel, UserEdgeLabel>::
node_del_cost_fun(const UserNodeLabel& node_label) const {
	//std::cout << "Called UserDefined node_del_cost_fun" << std::endl;

	double result;
	if (multiThreaded) {
		pybind11::gil_scoped_acquire acquire;
		result = pythonModule.attr("node_del_cost_fun")(node_label).template cast<double>();
		//result = pybind11::eval("edit_costs.node_del_cost_fun(" + std::to_string(node_label) + ")").template cast<double>();
	}
	else {
		result = pythonModule.attr("node_del_cost_fun")(node_label).template cast<double>();
		//result = pybind11::eval("edit_costs.node_del_cost_fun(" + std::to_string(node_label) + ")").template cast<double>();
	}
	return result;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
UserDefined<UserNodeLabel, UserEdgeLabel>::
node_rel_cost_fun(const UserNodeLabel& node_label_1, const UserNodeLabel& node_label_2) const {
	//std::cout << "Called UserDefined node_rel_cost_fun" << std::endl;

	double result;
	if (multiThreaded) {
		pybind11::gil_scoped_acquire acquire;
		result = pythonModule.attr("node_rel_cost_fun")(node_label_1, node_label_2).template cast<double>();
		//result = pybind11::eval("edit_costs.node_rel_cost_fun(" + std::to_string(node_label_1) + ", " + std::to_string(node_label_2) + ")").template cast<double>();
	}
	else {
		result = pythonModule.attr("node_rel_cost_fun")(node_label_1, node_label_2).template cast<double>();
		//result = pybind11::eval("edit_costs.node_rel_cost_fun(" + std::to_string(node_label_1) + ", " + std::to_string(node_label_2) + ")").template cast<double>();
	}
	return result;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
UserDefined<UserNodeLabel, UserEdgeLabel>::
edge_ins_cost_fun(const UserEdgeLabel& edge_label) const {
	//std::cout << "Called UserDefined edge_ins_cost_fun" << std::endl;

	double result;
	if (multiThreaded) {
		pybind11::gil_scoped_acquire acquire;
		result = pythonModule.attr("edge_ins_cost_fun")(edge_label).template cast<double>();
		//result = pybind11::eval("edit_costs.edge_ins_cost_fun(" + std::to_string(edge_label) + ")").template cast<double>();
	}
	else {
		result = pythonModule.attr("edge_ins_cost_fun")(edge_label).template cast<double>();
		//result = pybind11::eval("edit_costs.edge_ins_cost_fun(" + std::to_string(edge_label) + ")").template cast<double>();
	}
	return result;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
UserDefined<UserNodeLabel, UserEdgeLabel>::
edge_del_cost_fun(const UserEdgeLabel& edge_label) const {
	//std::cout << "Called UserDefined edge_del_cost_fun" << std::endl;

	double result;
	if (multiThreaded) {
		pybind11::gil_scoped_acquire acquire;
		result = pythonModule.attr("edge_del_cost_fun")(edge_label).template cast<double>();
		//result = pybind11::eval("edit_costs.edge_del_cost_fun(" + std::to_string(edge_label) + ")").template cast<double>();
	}
	else {
		result = pythonModule.attr("edge_del_cost_fun")(edge_label).template cast<double>();
		//result = pybind11::eval("edit_costs.edge_del_cost_fun(" + std::to_string(edge_label) + ")").template cast<double>();
	}
	return result;
}

template<class UserNodeLabel, class UserEdgeLabel>
double
UserDefined<UserNodeLabel, UserEdgeLabel>::
edge_rel_cost_fun(const UserEdgeLabel& edge_label_1, const UserEdgeLabel& edge_label_2) const {
	//std::cout << "Called UserDefined edge_rel_cost_fun" << std::endl;

	double result;
	if (multiThreaded) {
		pybind11::gil_scoped_acquire acquire;
		result = pythonModule.attr("edge_rel_cost_fun")(edge_label_1, edge_label_2).template cast<double>();
		//result = pybind11::eval("edit_costs.edge_rel_cost_fun(" + std::to_string(edge_label_1) + ", " + std::to_string(edge_label_2) + ")").template cast<double>();
	}
	else {
		result = pythonModule.attr("edge_rel_cost_fun")(edge_label_1, edge_label_2).template cast<double>();
		//result = pybind11::eval("edit_costs.edge_rel_cost_fun(" + std::to_string(edge_label_1) + ", " + std::to_string(edge_label_2) + ")").template cast<double>();
	}
	return result;
}

#endif /* SRC_UD_COSTS_IPP_ */

#endif /* SRC_UD_COSTS_HPP_ */
