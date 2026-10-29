node_ins_cost_fun = None
node_del_cost_fun = None
node_rel_cost_fun = None
edge_ins_cost_fun = None
edge_del_cost_fun = None
edge_rel_cost_fun = None

initialized = False


#   setup
def set_custom_edit_costs(node_ins_cost_fun_def,
                          node_del_cost_fun_def,
                          node_rel_cost_fun_def,
                          edge_ins_cost_fun_def,
                          edge_del_cost_fun_def,
                          edge_rel_cost_fun_def):
    global node_ins_cost_fun
    node_ins_cost_fun = node_ins_cost_fun_def
    global node_del_cost_fun
    node_del_cost_fun = node_del_cost_fun_def
    global node_rel_cost_fun
    node_rel_cost_fun = node_rel_cost_fun_def
    global edge_ins_cost_fun
    edge_ins_cost_fun = edge_ins_cost_fun_def
    global edge_del_cost_fun
    edge_del_cost_fun = edge_del_cost_fun_def
    global edge_rel_cost_fun
    edge_rel_cost_fun = edge_rel_cost_fun_def

    global initialized
    initialized = True
