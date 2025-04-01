import os
import matplotlib.pyplot
import scipy
import scipy.spatial
import scipy.cluster
import networkx
import glnx_parser
import edit_costs
#import lib.HGCDiCoDaGED as HGCDiCoDaGED
import lib.HGCGED as HGCGED


# ========== helper ==========
def _tree_to_graphnx_rec(tree, graph, labeling):

    if tree.left is not None:
        graph.add_node(tree.left.id, hgc_label=(labeling[tree.left.id] if (tree.left.left is None and tree.left.right is None) else str(tree.left.dist)))
        graph.add_edge(tree.id, tree.left.id)
        _tree_to_graphnx_rec(tree.left, graph, labeling)

    if tree.right is not None:
        graph.add_node(tree.right.id, hgc_label=(labeling[tree.right.id] if (tree.right.left is None and tree.right.right is None) else str(tree.right.dist)))
        graph.add_edge(tree.id, tree.right.id)
        _tree_to_graphnx_rec(tree.right, graph, labeling)


def _tree_to_graphnx(tree, labeling):
    graph = networkx.Graph()

    graph.add_node(tree.id, hgc_label=(labeling[tree.id] if (tree.left is None and tree.right is None) else str(tree.dist)))
    _tree_to_graphnx_rec(tree, graph, labeling)

    return graph


# rounds the labels of the given networkx graph to two decimals if they are numbers
def round_clustering_nx_labels(graph):
    labeling = []
    for x in range(len(graph.nodes)):
        try:
            element = graph.nodes[x]["hgc_label"]
        except KeyError:
            element = "N/A"
        try:
            element = str(round(float(element), 2))
        except ValueError:
            pass
        labeling.append(element)
    return dict(enumerate(labeling))


# ========== setup ==========
def set_custom_edit_costs(node_ins_cost_fun_def,
                          node_del_cost_fun_def,
                          node_rel_cost_fun_def,
                          edge_ins_cost_fun_def,
                          edge_del_cost_fun_def,
                          edge_rel_cost_fun_def):
    edit_costs.set_custom_edit_costs(node_ins_cost_fun_def,
                                     node_del_cost_fun_def,
                                     node_rel_cost_fun_def,
                                     edge_ins_cost_fun_def,
                                     edge_del_cost_fun_def,
                                     edge_rel_cost_fun_def)


class HGCEnv:

    #   variables
    _hgcged = None

    _label_list = None
    _distance_matrix = None

    _edit_costs = None
    _ged_method = None
    _clustering_algorithm = None

    _clustering = None
    _clustering_nx = None

    # ========== setup ==========

    # constructs the hgc environment
    def __init__(self, ged_method='', method_arguments='', use_custom_edit_costs=False, init_type=''):
        if use_custom_edit_costs and not edit_costs.initialized:
            raise Exception("Custom edit costs were activated but not gml up before. Use hgc_env.set_custom_edit_costs(func, func, func, func, func, func).")
        try:
            self._hgcged = HGCGED.HGCGED(ged_method, method_arguments, use_custom_edit_costs, init_type)
        except ModuleNotFoundError:
            raise ModuleNotFoundError("Couldn't find edit_costs.py file! Please stick to the following file structure:\n"
                                      "root\n"
                                      "\t- lib\n"
                                      "\t\t- libdicodagedlib.so\n"
                                      "\t\t- dicodaGed.so\n"
                                      "\t\t- ...\n"
                                      "\t- edit_costs.py\n"
                                      "\t- glnx_parser.py\n"
                                      "\t- hgc.py\n"
                                      "\t- main.py\n"
                                      "\t- ...")
        self._edit_costs = self._hgcged.get_edit_costs_name()
        self._ged_method = self._hgcged.get_method_name()

    # calls the set_method method of hgcged (enabling to gml the method anew)
    def set_ged_method(self, ged_method, method_arguments=''):
        self._hgcged.set_method(ged_method, method_arguments)
        self._ged_method = self._hgcged.get_method_name()

    def load_csv(self, omics_dataset, clinical_dataset='', costs_dataset='', use_csv_edit_costs=False, seperator=','):
        print('Loading CSV data... ', end='')
        self._hgcged.load_csv(omics_dataset, clinical_dataset, costs_dataset, use_csv_edit_costs, seperator)
        if use_csv_edit_costs:
            self._edit_costs = self._hgcged.get_edit_costs_name()
        print('Done!')

    # ========== cluster ==========

    # calls the generate_labels method of hgcged and saves the result
    def generate_labels(self, labeled_variable=''):
        print('Generating labels... ', end='')
        self._hgcged.generate_labels(labeled_variable)
        self._label_list = self._hgcged.get_label_vector()
        if len(self._label_list) == 0:
            self._label_list = None
        print('Done!')

    # calls the compute_geds method of hgcged and saves the result
    def compute_geds(self):
        if self._ged_method is None:
            raise TypeError("GED method is undefined!")

        print('Calculating graph edit distances (using the ' + self._ged_method + ' method with ' + self._edit_costs + ' edit costs)...')
        self._hgcged.compute_geds()
        self._distance_matrix = self._hgcged.get_distance_matrix()
        if len(self._distance_matrix) == 0:
            self._distance_matrix = None
        print('Done!')

    # generates the clustering, using the ged matrix obtained by compute_geds, and saves it
    def generate_clustering(self, algorithm=''):
        if self._distance_matrix is None:
            raise TypeError("Distance matrix is undefined (or empty)!")

        fixed_algorithm = None
        if algorithm == '':
            print("No clustering algorithm passed. Defaulting to \"upgma\".")
            fixed_algorithm = 'average'
        elif algorithm == 'diana':
            raise NotImplementedError('Only bottom-up clustering (AGNES) is available at the moment.')
        elif algorithm == 'agnes':
            raise Exception("Specific bottom-up clustering algorithm needed. The following are available:\n"
                            "\t\tnearest point, farthest point, upgma, wpgma, upgmc, wpgmc, incremental")
        else:
            if algorithm == 'nearest point':
                fixed_algorithm = 'single'
            if algorithm == 'farthest point':
                fixed_algorithm = 'complete'
            if algorithm == 'upgma':
                fixed_algorithm = 'average'
            if algorithm == 'wpgma':
                fixed_algorithm = 'weighted'
            if algorithm == 'upgmc':
                fixed_algorithm = 'centroid'
            if algorithm == 'wpgmc':
                fixed_algorithm = 'median'
            if algorithm == 'incremental':
                fixed_algorithm = 'ward'
        if fixed_algorithm != 'single' and fixed_algorithm != 'complete' and fixed_algorithm != 'average' and fixed_algorithm != 'weighted' and fixed_algorithm != 'centroid' and fixed_algorithm != 'median' and fixed_algorithm != 'ward':
            raise Exception("Invalid clustering algorithm passed (\"" + algorithm + "\").")

        self._clustering_algorithm = fixed_algorithm
        print('Generating clustering (using the ' + self._clustering_algorithm + ' method)... ', end='')
        condensed_matrix = scipy.spatial.distance.pdist(self._distance_matrix)
        self._clustering = scipy.cluster.hierarchy.linkage(condensed_matrix, method=self._clustering_algorithm, optimal_ordering=True)
        print('Done!')

    # generates the networkx graph of the clustering and saves it
    def generate_clustering_nx(self):
        if self._clustering is None:
            raise TypeError("Clustering is undefined!")
        if self._label_list is None:
            self.generate_labels()

        print('Converting clustering to NetworkX graph... ', end='')
        clustering_tree = scipy.cluster.hierarchy.to_tree(self._clustering)
        self._clustering_nx = _tree_to_graphnx(clustering_tree, self._label_list)
        print('Done!')

    # ========== save ==========

    # generates the dendrogram plot for the clustering
    def plot_clustering(self, out_dir, save=True, show=False):
        if self._clustering is None:
            raise TypeError("Clustering is undefined!")
        if self._label_list is None:
            self.generate_labels()

        matplotlib.pyplot.figure(figsize=(25, 10))
        print('Plotting dendrogram' + (' & saving plot' if save else '') + '...', end='')
        scipy.cluster.hierarchy.dendrogram(self._clustering, labels=self._label_list, leaf_rotation=90)
        if save:
            try:
                os.mkdir(out_dir + 'dendrogram/')
            except FileExistsError:
                pass
            matplotlib.pyplot.savefig(out_dir + 'dendrogram/' + self._get_filename() + ".png", dpi=300, bbox_inches='tight')
        if show:
            matplotlib.pyplot.show()
        else:
            matplotlib.pyplot.close()
        print('Done!')

    # generates the plot of the networkx graph clustering
    def plot_clustering_nx(self, out_dir, save=True, show=False):
        if self._clustering_nx is None:
            raise TypeError("Clustering graph is undefined!")

        matplotlib.pyplot.figure(figsize=(35, 14))
        print('Plotting graph' + (' & saving plot' if save else '') + '...', end='')
        networkx.draw_networkx(self._clustering_nx, pos=networkx.spring_layout(self._clustering_nx, seed=0), with_labels=True, labels=round_clustering_nx_labels(self._clustering_nx), font_size=6, node_size=500, node_color="cyan", edge_color="gray")
        if save:
            try:
                os.mkdir(out_dir + 'clustering/')
            except FileExistsError:
                pass
            matplotlib.pyplot.savefig(out_dir + 'clustering/' + self._get_filename() + ".png", dpi=300, bbox_inches='tight')
        if show:
            matplotlib.pyplot.show()
        else:
            matplotlib.pyplot.close()
        print('Done!')

    # saves the networkx graph of the clustering as a gml file
    def save_clustering_nx_gml(self, out_dir):
        if self._clustering_nx is None:
            raise TypeError("Clustering graph is undefined")

        try:
            os.mkdir(out_dir + 'gml/')
        except FileExistsError:
            pass
        print('Saving graph gml... ', end='')
        networkx.write_graphml(self._clustering_nx, out_dir + 'gml/' + self._get_filename() + ".gml")
        print('Done!')

    # ========== helper ==========

    # calls all necessary functions consecutively
    def run_clustering_full(self, out_dir, algorithm=''):
        self.generate_clustering(algorithm)
        self.generate_clustering_nx()
        self.plot_clustering(out_dir, save=True, show=False)
        self.plot_clustering_nx(out_dir, save=True, show=False)
        self.save_clustering_nx_gml(out_dir)

    # generates a filename out of the clustering and ged computation methods in use
    def _get_filename(self):
        if self._clustering_algorithm is None:
            raise TypeError("Clustering algorithm is undefined!")
        if self._ged_method is None:
            raise TypeError("GED method is undefined!")

        return self._clustering_algorithm + '_' + self._ged_method

    # ========== getter ==========

    # returns the saved clustering
    def get_clustering(self):
        return self._clustering

    # returns the saved networkx graph of the clustering
    def get_clustering_nx(self):
        return self._clustering_nx

    # ========== export & import ==========

    def pull_graph(self, graph_id):
        return glnx_parser.pull_graph(self._hgcged, graph_id)

    def push_graph(self, graph, name, node_label_key='', edge_label_key=''):
        if node_label_key == '':
            print("No node label key passed. Defaulting to \"hgc_node_label\".")
            node_label_key = 'hgc_node_label'
        if edge_label_key == '':
            print("No edge label key passed. Defaulting to \"hgc_edge_label\".")
            edge_label_key = 'hgc_edge_label'
        return glnx_parser.push_graph(self._hgcged, graph, name, node_label_key, edge_label_key)

    def export_environment(self, out_dir):
        print('Exporting environment to \"' + out_dir + '\"... ', end='')
        glnx_parser.export_environment(self._hgcged, out_dir)
        print('Done!')

    def import_environment(self, directory, node_label_key='', edge_label_key=''):
        if node_label_key == '':
            print("No node label key passed. Defaulting to \"hgc_node_label\".")
            node_label_key = 'hgc_node_label'
        if edge_label_key == '':
            print("No edge label key passed. Defaulting to \"hgc_edge_label\".")
            edge_label_key = 'hgc_edge_label'
        print('Importing environment from \"' + directory + '\"... ', end='')
        glnx_parser.import_environment(self._hgcged, directory, node_label_key, edge_label_key)
        print('Done!')
