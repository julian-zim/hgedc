#python3 main.py -help

python main.py -out out/ -gml ../data/gml/ -csv_omics ../data/csv/test_otu_abundances.csv -csv_clinical ../data/csv/test_clinical_data.csv -csv_distances ../data/csv/test_otu_distances.csv

#-out out/
#-gml out/export/
#-gml_node_label hgc_node_label
#-gml_edge_label hgc_edge_label
#-csv_omics ../data/test_otu_abundances.csv
#-csv_clinical ../data/test_clinical_data.csv
#-csv_distances ../data/test_otu_distances.csv
#-edit_costs constant|custom|auto
#-label_attribute HSCT_responder
#-cluster_algo nearest_point|farthest_point|upgma|wpgma|upgmc|wpgmc|incremental
#-ged_method SUPER_FAST|STANDARD|TIGHT
#--threads 10
#-init_type LAZY|EAGER
