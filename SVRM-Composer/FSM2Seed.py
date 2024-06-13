from pydot import *
import argparse
import os
import glob
from StateMachine import StateMachine
import hashlib

def zhuliu_DMST (automata:StateMachine)->list:
    ans = 0
    root_node = 0
    edges_src = []
    edges_dst = []
    edges_wt = []    
    for edge in automata.edges:
        
        edges_src.append(edge.src)
        edges_dst.append(edge.dst)
        edges_wt.append(edge.weight)
    edge_number = automata.edges.__len__()    
    cur_node_number = automata.nodes.__len__()
    
    node_inedge_min_weight = [1e+9] * cur_node_number
    loop = [0] * cur_node_number
    travel_start_pos = [-1] * cur_node_number
    father = [-1] * cur_node_number
    selected_edge = []
    tot = 0 
    while True:
        for i in range(0, cur_node_number):
            node_inedge_min_weight[i] = 1e+9
        for i in range (0, edge_number):
            if edges_src[i] != edges_dst[i]: 
                w = edges_wt[i]
                dst_node = edges_dst[i]
                cur_dst_node_w = node_inedge_min_weight[dst_node]
                if w < cur_dst_node_w : 
                    if edges_dst[i] != root_node:
                        selected_edge.append(i)
                    
                    node_inedge_min_weight[dst_node] = w 
                    father[dst_node] = edges_src[i] 
                    
        node_inedge_min_weight[0]=0; 
        for i in range(0, cur_node_number):
            ans = ans + node_inedge_min_weight[i]
            if node_inedge_min_weight[i] == 1e+9:
                return []
        node_v = 0
        for node_u in range(0, cur_node_number ):    
            node_v = node_u      
            while node_v != root_node and travel_start_pos[node_v] != node_u and loop[node_v] == 0:
                travel_start_pos[node_v] = node_u 
                node_v = father[node_v]
            if node_v != root_node and loop[node_v] == 0:
                tot = tot+1
                loop[node_v] = tot
                node_k = father[node_v]
                while node_k != node_v:
                    loop[node_k] = tot
                    node_k = father[node_k]
            
        if tot == 0:
            return selected_edge
        for node_i in range(0, cur_node_number ):
            if loop[node_i] == 0:
                tot = tot+1
                loop[node_i] = tot
                
        for edge_i in range(0, edge_number):         
            edges_src[edge_i] = loop[edges_src[edge_i]]
            edges_dst[edge_i] = loop[edges_dst[edge_i]]
            edges_wt[edge_i] = edges_wt[edge_i] - node_inedge_min_weight[edges_dst[edge_i]]
            cur_node_number = tot
            root_node = loop[root_node]
            tot = 0

       
def gerenrating_DMST_path(automata:StateMachine, selected_edges_number:list)->list:
    selected_edges = []
    
    for i in selected_edges_number:
        selected_edges.append(automata.edges[i])    
    wokrlist = []
    path_to_all = [[]]*len(automata.nodes)
    wokrlist.append(0)
    while len(wokrlist) != 0:
        # 找到所有从当前节点出发的边：
        cur_node = wokrlist.pop()
        for edge_j in selected_edges:
            if edge_j.src == cur_node:
                dst_node = edge_j.dst
                wokrlist.append(dst_node)
                father_list = path_to_all[cur_node]
                new_list = father_list[:]
                new_list.append(edge_j)
                path_to_all[dst_node] = new_list               
    return path_to_all
        
def expand_path(automata:StateMachine,basic_path:list,selected_edges_number:list)->list:
    all_seed_need = []
    dead_nodes = []
    main_edges = []
    for i in selected_edges_number:
        main_edges.append(automata.edges[i])
        
    for j in range(0,len(automata.nodes)):
        is_dead_node = True
        if len(automata.nodes[j].loop_edges) == 0:
            is_dead_node = False
            continue
        for lp in automata.nodes[j].loop_edges:
            if lp.hash != 'ConnectionClosed':
                is_dead_node = False
                break       
        if is_dead_node == True:
            dead_nodes.append(j)   
             
    is_leave_node = [True]*len(automata.nodes)
    for k in range(0,len(main_edges)):
        is_leave_node[main_edges[k].src] = False
       
    for node in automata.nodes:
        node_number = int(re.sub("s","",node.get_name()))
        main_path = basic_path[node_number]  
        if node_number in dead_nodes:
            continue
        
        if is_leave_node[node_number] == True:
            all_seed_need.append(main_path)
        
        for o_edge in node.out_edges:
            if main_edges.count(o_edge) != 0:
                continue
            new_list = main_path[:]
            new_list.append(o_edge)           
            all_seed_need.append(new_list)               
    return all_seed_need
     
def expand_path_all(automata:StateMachine,basic_path:list,selected_edges_number:list,stateful_msg)->list:
    all_seed_need = []
    dead_nodes = []
    main_edges = []
    for i in selected_edges_number:
        main_edges.append(automata.edges[i])
        
    for j in range(0,len(automata.nodes)):
        is_dead_node = True
        if len(automata.nodes[j].loop_edges) > 0:
            for lp in automata.nodes[j].loop_edges:
                if lp.hash != 'ConnectionClosed':
                    is_dead_node = False
                    break       
            if is_dead_node == True:
                dead_nodes.append(j)
        else:
            is_dead_node = False
            
    is_leave_node = [True]*len(automata.nodes)
    for k in range(0,len(main_edges)):
        is_leave_node[main_edges[k].src] = False
         
    for node in automata.nodes:
        node_number = int(re.sub("s","",node.get_name()))
        if node_number == 0:
            continue        
        if node_number in dead_nodes:
            continue
        main_path = basic_path[node_number]          
        if is_leave_node[node_number] == True:
            all_seed_need.append(main_path)
        
        for o_edge in node.out_edges:
            if main_edges.count(o_edge) != 0:
                continue
            if o_edge.dst in dead_nodes:
                continue
            
            new_list = main_path[:]
            new_list.append(o_edge)           
            all_seed_need.append(new_list)  
        
                
                        
    return all_seed_need
 
def expand_path_all_trial(automata:StateMachine,basic_path:list,selected_edges_number:list)->list:
    all_seed_need = []
    dead_nodes = []
    main_edges = []
    for i in selected_edges_number:
        main_edges.append(automata.edges[i])
        
    for j in range(0,len(automata.nodes)):
        is_dead_node = True
        if len(automata.nodes[j].loop_edges) > 0:
            for lp in automata.nodes[j].loop_edges:
                if lp.hash != 'ConnectionClosed':
                    is_dead_node = False
                    break       
            if is_dead_node == True:
                dead_nodes.append(j)
        else:
            is_dead_node = False
            
    is_leave_node = [True]*len(automata.nodes)
    for k in range(0,len(main_edges)):
        is_leave_node[main_edges[k].src] = False
    
    for node in automata.nodes:
        node_number = int(re.sub("s","",node.get_name()))
        if node_number == 0:
            continue        
        main_path = basic_path[node_number]          
        all_seed_need.append(main_path)
                    
    return all_seed_need    

def get_stateful_edge(automata:StateMachine)->list:
    in_out_edges_sym = []
    for edge in automata.edges:
        if edge.is_selfloop == True:
            continue
        in_out_edges_sym.append(edge.in_sym)
    return set(in_out_edges_sym)
            
def DMST_generating_path(automata:StateMachine)->list:
    selected_edges_number = zhuliu_DMST(automata)
    basic_path = gerenrating_DMST_path(automata, selected_edges_number)
    stateful_msg = get_stateful_edge(automata)
    result = expand_path(automata,basic_path,selected_edges_number)
    # result = expand_path_all(automata,basic_path,selected_edges_number,stateful_msg)
    # result = expand_path_all_trial(automata,basic_path,selected_edges_number,stateful_msg)
    return result

def get_message_corpus(messages_dir:str)->dict:
    msg_dic = {}
    os.chdir(messages_dir)
    raw_list = glob.glob('*.raw')
    raw_list += glob.glob('*.RAW')
    for rawfile in raw_list:       
        with open(rawfile,"rb") as file:
            fname = os.path.basename(rawfile)
            symbol = fname.split('.')[0].upper()
            content = file.read()
            msg_dic[symbol]=content    
    return msg_dic


def generating_raw(sym_seq:list,sym_dict:dict,used_symbol_dict:set,out_put_path:str):
    seed_content = b""
    for sym in sym_seq:
        seed_content+=sym_dict[sym]
        used_symbol_dict.add(sym)
        
    print(sym_seq)
    hash_name =  hashlib.md5(seed_content).hexdigest()
    file_path = os.path.join(out_put_path,hash_name+".raw")
    with open(file_path,'wb') as f:
        f.write(seed_content)
    
def main(dotfile:str, message_corpus:str, out_path:str):
    folder = os.path.exists(out_path)
    if not folder:
        os.makedirs(out_path)
    Files=os.listdir(out_path)
    for k in range(len(Files)):
        if ".raw" in os.path.splitext(Files[k])[1]:
            os.remove(out_path+'/'+Files[k])
    statemachine = StateMachine(dotfile)
    # statemachine.print_stat()
    msg_dic = get_message_corpus(message_corpus)
    used_symbol_dict = set()
    edge_seqs = DMST_generating_path(statemachine)

    for edge_seq in edge_seqs:
        has_connection_closed = False
        sym_seq = []
        for edge in edge_seq:
            if edge.hash == "ConnectionClosed":
                has_connection_closed = True
                continue
            if has_connection_closed == False:
                sym_seq.append(edge.in_sym)
        generating_raw(sym_seq,msg_dic,used_symbol_dict,out_path)
    
    for msg in msg_dic:
        if msg in used_symbol_dict:
            continue
        else:
            seed_content = b""
            seed_content+=msg_dic[msg]
            print(msg)
            hash_name =  hashlib.md5(seed_content).hexdigest()
            file_path = os.path.join(out_path,hash_name+".raw")
            with open(file_path,'wb') as f:
                f.write(seed_content)

    statemachine.print_SM_infos()
    
    

if __name__ == '__main__':
    parser = argparse.ArgumentParser() 
    parser.add_argument('-a','--automata',type=str,required=True,help="Full path to automata (dot file)")
    parser.add_argument('-m','--messages',type=str,required=True,help="Message corpus")
    parser.add_argument('-o','--outpath',type=str,required=True,help="output path")
    args = parser.parse_args()
    main(args.automata, args.messages,args.outpath)