import sys
import scipy
import scipy.spatial
from scipy.cluster.hierarchy import dendrogram, linkage
from matplotlib import pyplot
import lib.cppBindings_gxl as dicodaGedEnv


def parse_file(path):
    file = open(path + ".csv", "r")

    matrix = [[]]

    current_row = "0"
    new_row = ""

    current_value = ""
    current_char = file.read(1)

    while current_char != "":
        while current_char != "x":
            new_row += current_char
            current_char = file.read(1)
        if int(current_row) != int(new_row):
            current_row = new_row
            matrix.append([])
        new_row = ""
        while current_char != "_":
            current_char = file.read(1)
        current_char = file.read(1)
        while current_char != "_":
            current_value += current_char
            current_char = file.read(1)
        matrix[int(current_row)].append(int(current_value))
        current_value = ""
        current_char = file.read(1)
        while current_char != "\n":
            current_char = file.read(1)
        current_char = file.read(1)

    return matrix


def symmetrize_matrix(matrix):
    for x in range(len(matrix)):
        for y in range(len(matrix[x]))[x+1:]:
            first = matrix[x][y]
            second = matrix[y][x]
            if first > second:
                matrix[x][y] = second
            else:
                matrix[y][x] = first

    return matrix


def construct_clustering(raw_matrix):
    symmetric_matrix = symmetrize_matrix(raw_matrix)
    condensed_matrix = scipy.spatial.distance.pdist(symmetric_matrix)
    clustering = linkage(condensed_matrix, method='average')
    return clustering


def ged(outfile, dataset_path, method, edit_costs, init_type, method_arguments):
    print("Running pybind11 test... ", end="")
    if dicodaGedEnv.pybind_test(1, 2) == 3:
        print("Success!")
        print("Running GED...")
        if outfile:
            dicodaGedEnv.run_with_outfile(dataset_path, method, edit_costs, init_type, method_arguments)
        else:
            return dicodaGedEnv.run_with_return(dataset_path, method, edit_costs, init_type, method_arguments)
    else:
        raise Exception("Error!")


def make_dendrogram(clustering, filename, show):
    pyplot.figure(figsize=(25, 10))
    dendrogram(clustering)
    pyplot.savefig(filename + ".png", dpi=300, bbox_inches='tight')
    if show:
        pyplot.show()


def parse_arguments(raw_arguments):
    usage_string = "Usage: python3 run_on_dataset.py " \
                   "-d <dataset> " \
                   "[-m SUPER_FAST|FAST|TIGHT] " \
                   "[-ec CONSTANT] " \
                   "[-it LAZY/EAGER] " \
                   "[--<method-option> <method-arg>] [...]"

    arguments = [""] * 5

    c = 1
    while c < len(raw_arguments):
        if raw_arguments[c][0] == '-':
            if c+1 < len(raw_arguments):
                if raw_arguments[c][1:] == "d":
                    arguments[0] = raw_arguments[c+1]
                elif raw_arguments[c][1:] == "m":
                    arguments[1] = raw_arguments[c+1]
                elif raw_arguments[c][1:] == "ec":
                    arguments[2] = raw_arguments[c+1]
                elif raw_arguments[c][1:] == "it":
                    arguments[3] = raw_arguments[c+1]
                elif raw_arguments[c][1] == '-':
                    if arguments[4] != "":
                        arguments[4] += " "
                    arguments[4] += raw_arguments[c] + " " + raw_arguments[c+1]
                else:
                    raise Exception("Invalid option \"" + raw_arguments[c][1:] + "\". " + usage_string)
                c += 1
            else:
                raise Exception("Option \"" + raw_arguments[c][1:] + "\" specified but no argument given. "
                                + usage_string)
        else:
            print("Ignoring argument \"" + raw_arguments[c] + "\" since there is no option specified for it.")
        c += 1

    if arguments[0] == "":
        raise Exception("Dataset not specified. " + usage_string)

    return arguments


def main():
    out_dir = "out/"

    #   parse
    arguments = parse_arguments(sys.argv)

    #   GED
    outfile = True
    if outfile:
        ged(True, "../datasets/" + arguments[0], arguments[1], arguments[2], arguments[3], arguments[4])
        filename = dicodaGedEnv._get_filename()
        result_list = parse_file(out_dir + filename)
    else:
        result_list = ged(False, "../datasets/" + arguments[0], arguments[1], arguments[2], arguments[3], arguments[4])
        filename = dicodaGedEnv._get_filename()

    #   cluster
    print("Clustering results...")
    clustering = construct_clustering(result_list)
    make_dendrogram(clustering, out_dir + filename, False)
    print("Done! (TODO: Fix the now occuring segmentation fault.)")


main()
