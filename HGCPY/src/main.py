import sys
import os
import hgc_env


#   variables
out_path = None
gml_path = None
gml_node_label_key = None
gml_edge_label_key = None
csv_omics_path = None
csv_clinical_path = None
csv_distances_path = None
edit_costs = None
labeled_attribute = None
cluster_algorithm = None
ged_method = None
method_arguments = None
init_type = None


#   USER COST FUNCTIONS -------------------------------
def node_ins_cost(label):
    print("Using template custom node insertion cost function with " + str(label) + " as input.")
    return 1.0


def node_del_cost(label):
    print("Using template custom node deletion cost function with " + str(label) + " as input.")
    return 1.0


def node_rel_cost(label1, label2):
    print("Using template custom node relabeling cost function with " + str(label1) + " and " + str(label2) + " as input.")
    if label1 == label2:
        return 0.0
    return 1.0


def edge_ins_cost(label):
    print("Using template custom edge insertion cost function with " + str(label) + " as input.")
    return 1.0


def edge_del_cost(label):
    print("Using template custom edge deletion cost function with " + str(label) + " as input.")
    return 1.0


def edge_rel_cost(label1, label2):
    print("Using template custom edge relabeling cost function with " + str(label1) + " and " + str(label2) + " as input.")
    if label1 == label2:
        return 0.0
    return 1.0
#   ---------------------------------------------------


#   helper
def parse_arguments(raw_arguments):
    usage_string = "Usage:\tpython3 main.py\n" \
                   "\t-out <path-to-out-directory>\n" \
                   "\t-gml <path-to-directory>\n" \
                   "\t[-gml_node_label <node-label-key>]\n" \
                   "\t[-gml_edge_label <node-label-key>]\n" \
                   "\t-csv_omics <path-to-csv-file>\n" \
                   "\t[-csv_clinical <path-to-csv-file>]\n" \
                   "\t[-csv_distances <path-to-csv-file>]\n" \
                   "\t[-edit_costs constant|custom|auto]\n" \
                   "\t[-label_attribute <clinical-attribute>]\n" \
                   "\t[-cluster_algo nearest_point|farthest_point|upgma|wpgma|upgmc|wpgmc|incremental]\n" \
                   "\t[-ged_method SUPER_FAST|FAST|TIGHT]\n" \
                   "\t[--<method-option> <method-arg>] [...]\n" \
                   "\t[-init_type LAZY|EAGER]\n" \
                   "If GML data is specified, CSV data can be omitted, and vice-versa." \

    global out_path
    out_path = ''
    global gml_path
    gml_path = ''
    global gml_node_label_key
    gml_node_label_key = ''
    global gml_edge_label_key
    gml_edge_label_key = ''
    global csv_omics_path
    csv_omics_path = ''
    global csv_clinical_path
    csv_clinical_path = ''
    global csv_distances_path
    csv_distances_path = ''
    global edit_costs
    edit_costs = ''
    global labeled_attribute
    labeled_attribute = ''
    global cluster_algorithm
    cluster_algorithm = ''
    global ged_method
    ged_method = ''
    global method_arguments
    method_arguments = ''
    global init_type
    init_type = ''

    if len(raw_arguments) < 2:
        print(usage_string)
        quit()

    c = 1
    while c < len(raw_arguments):
        if raw_arguments[c][0] == '-':      # if option
            if raw_arguments[c][1:] == "help":  # if option help
                print(usage_string)
                quit()
            if c + 1 < len(raw_arguments):      # if argument
                if len(raw_arguments[c]) > 1:      # if option not nameless
                    if raw_arguments[c][1:] == "out":
                        out_path = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "gml":
                        gml_path = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "gml_node_label":
                        gml_node_label_key = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "gml_edge_label":
                        gml_edge_label_key = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "csv_omics":
                        csv_omics_path = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "csv_clinical":
                        csv_clinical_path = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "csv_distances":
                        csv_distances_path = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "edit_costs":
                        edit_costs = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "label_attribute":
                        labeled_attribute = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "cluster_algo":
                        cluster_algorithm = raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "ged_method":
                        ged_method = raw_arguments[c + 1]
                    elif raw_arguments[c][1] == '-':
                        if method_arguments != "":
                            method_arguments += " "
                        method_arguments += raw_arguments[c] + " " + raw_arguments[c + 1]
                    elif raw_arguments[c][1:] == "init_type":
                        init_type = raw_arguments[c + 1]
                    else:
                        raise Exception("Invalid option \"" + raw_arguments[c][1:] + "\".\n" + usage_string)
                    c += 1
                else:
                    raise Exception("Empty option at position " + str(c) + ".\n" + usage_string)
            else:
                raise Exception("Option \"" + raw_arguments[c][1:] + "\" specified but no argument given for it.\n" + usage_string)
        else:
            raise Exception("Argument \"" + raw_arguments[c] + "\" given but no option specified for it.\n" + usage_string)
        c += 1

    #   out directory
    try:
        os.mkdir(out_path)
    except FileExistsError:
        pass

    #   dataset check
    if gml_path == '' and csv_omics_path == '':
        raise Exception("A GML or a CSV omics dataset must be specified.")

    #   label key warning
    if gml_path == '' and (gml_edge_label_key != '' or gml_edge_label_key != ''):
        print("Warning: No GML dataset was specified, but label keys were.")

    #   edit costs check
    if edit_costs == "":
        print("No edit costs passed. Defaulting to \"auto\".")
        edit_costs = "auto"
    elif edit_costs != "constant" and edit_costs != "custom" and edit_costs != "auto":
        raise Exception("Invalid edit_costs passed (\"" + edit_costs + "\").")

    #   clustering algorithm precheck
    if cluster_algorithm == 'nearest_point':
        cluster_algorithm = 'nearest point'
    if cluster_algorithm == 'farthest_point':
        cluster_algorithm = 'farthest point'
    if cluster_algorithm != '' \
            and cluster_algorithm != 'nearest point' \
            and cluster_algorithm != 'single' \
            and cluster_algorithm != 'farthest point' \
            and cluster_algorithm != 'complete' \
            and cluster_algorithm != 'upgma' \
            and cluster_algorithm != 'average' \
            and cluster_algorithm != 'wpgma' \
            and cluster_algorithm != 'weighted' \
            and cluster_algorithm != 'upgmc' \
            and cluster_algorithm != 'centroid' \
            and cluster_algorithm != 'wpgmc' \
            and cluster_algorithm != 'median' \
            and cluster_algorithm != 'incremental' \
            and cluster_algorithm != 'ward':
        raise Exception("Invalid clustering algorithm passed (\"" + cluster_algorithm + "\").")


#   main
def run():

    #   parse
    parse_arguments(sys.argv)

    #   edit costs
    if edit_costs == "custom":
        hgc_env.set_custom_edit_costs(node_ins_cost, node_del_cost, node_rel_cost, edge_ins_cost, edge_del_cost, edge_rel_cost)

    #   construct
    hgc = hgc_env.HGCEnv(ged_method, method_arguments, True if edit_costs == "custom" else False, init_type)

    #   csv
    if csv_omics_path != '':
        hgc.load_csv(csv_omics_path, csv_clinical_path, csv_distances_path, True if edit_costs == "auto" else False)

    #   gml
    if gml_path != '':
        hgc.import_environment(gml_path, gml_node_label_key, gml_edge_label_key)

    #   label
    hgc.generate_labels(labeled_attribute)

    #   ged
    hgc.compute_geds()

    #   cluster
    hgc.run_clustering_full(out_path, cluster_algorithm)


run()
