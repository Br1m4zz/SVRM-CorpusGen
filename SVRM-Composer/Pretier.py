import pydot
import argparse
def pretty(path:str):
    graphs = pydot.graph_from_dot_file(path)
    graph = graphs[0]
    newgraph = pydot.Dot(graph_type='digraph')
    node_num = len(graph.get_nodes())
    edge_num = len(graph.get_edges())
    for pydot_node in graph.get_nodes():
        newgraph.add_node(pydot_node)
        
    
    for pydot_edge in graph.get_edges():
        if pydot_edge.get_source() != pydot_edge.get_destination():
            newgraph.add_edge(pydot_edge)
            
    for pydot_edge in graph.get_edges():
        src = pydot_edge.get_source()
        dst = pydot_edge.get_destination()
        print(src+"-->"+dst)
        
    print(node_num)
    print(edge_num)
        
        
    newgraph.write_dot(path+'.prety.dot')
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser() 
    parser.add_argument('-i','--input',type=str,required=True,help="Full path to automata (dot file)")
    args = parser.parse_args()
    pretty(args.input)