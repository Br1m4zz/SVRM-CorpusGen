import re
import pydot

class SM_node:
        def __init__(self, node_name:str):
            self.name = node_name
            self.out_edges=[]
            self.in_edges=[]
            self.loop_edges=[]
        def get_name(self):
            return self.name
        
        def add_outedge(self,edge:object):
            self.out_edges.append(edge)
    
        def add_inedge(self,edge:object):
            self.in_edges.append(edge)
            
        def add_loopedge(self,edge:object):
            self.loop_edges.append(edge)
            
class SM_edge:
        def __init__(self, in_symbol:str, state:str, start_node:SM_node,end_node:SM_node):
            self.in_sym = in_symbol
            self.hash = state
            self.weight = 1
            self.src_node = start_node
            self.src = int(re.sub("s","",start_node.get_name())) #仅带数字
            self.dst_node = end_node
            self.dst = int(re.sub("s","",end_node.get_name())) #仅带数字
            
            self.is_selfloop = False
            if self.src_node == self.dst_node:
                self.is_selfloop = True

        def get_insym(self):
            return self.in_sym

        def get_state(self):
            return self.hash

        def get_src(self):
            return self.src_node

        def get_dst(self):
            return self.dst_node    
        


class StateMachine:    
    def __init__(self,dotfilepath:str):
        self.nodes=[]
        self.edges=[]
        self.root_node = None
        self.loop_edges=[]
        graphs = pydot.graph_from_dot_file(dotfilepath)
        graph = graphs[0]
        pydot_edges = graph.get_edge_list()
        pydot_nodes = graph.get_node_list()
        #解析dot文件，先把所有节点保存
        for pydot_node in pydot_nodes:
            node_name = pydot_node.get_name()
            match = re.match(r"s\d+", node_name)
            if match:
                cur_node = SM_node(node_name)
                self.nodes.append(cur_node)
                if node_name == "s0":
                    self.root_node = cur_node
        #解析dot文件的边
        for pydot_edge in pydot_edges:
            pydot_node_src = pydot_edge.get_source()
            pydot_node_dst = pydot_edge.get_destination()
            #忽略起始
            if pydot_node_src == "__start0":
                continue
            #获取边的标志信息
            attr = pydot_edge.get_attributes()['label']
            SYM = attr.split("/")[0].strip('\"').strip(' ')
            STT = attr.split("/")[1].strip('\"').strip(' ')
            
            #从SM已知节点找到对应的节点（根据名称，记录内存存储值）
            for node in self.nodes:
                if node.name == pydot_node_src:
                    src_node = node   
                if node.name == pydot_node_dst:
                    dst_node = node
            
            #新建这个边
            newedge = SM_edge(SYM,STT,src_node,dst_node)
            self.edges.append(newedge)
            
            #更新边对应的节点的父子信息
            for node in self.nodes:
                #loop
                if newedge.is_selfloop == True and node == src_node:
                    node.add_loopedge(newedge)
                    self.loop_edges.append(newedge)
                #children
                elif node == src_node and src_node != dst_node:
                    node.add_outedge(newedge)   
                #parent
                elif node == dst_node and src_node != dst_node:
                    node.add_inedge(newedge)
            
            
    def get_edge_list(self):
        return self.edges
    
    def get_node_list(self):
        return self.nodes
    
    def print_stat(self):
        print("****nodes****")
        for iter_node in self.nodes:
            print("------------------")
            print("name:"+iter_node.get_name())
            print("parents:")
            for iter in iter_node.in_edges:
                print(iter.get_src().name)
            print("children:")
            for iter in iter_node.out_edges:
                print(iter.get_dst().name)
            print("Loops:")
            for iter in iter_node.loop_edges:
                print(iter.get_dst().name)
        
        print("****edges****")    
        for iter_egde in self.edges:
            print(iter_egde.get_src().get_name(),"->",iter_egde.get_dst().get_name(),":",iter_egde.get_insym(),"/",iter_egde.get_state())
            
    def print_SM_infos(self):
        node_number = len(self.nodes)
        edge_number = len(self.edges)
        loop_edge_number = len(self.loop_edges)
        print("nodes#:"+str(node_number))
        print("edge#:"+str(edge_number))
        print("loop_edge#:"+str(loop_edge_number))
