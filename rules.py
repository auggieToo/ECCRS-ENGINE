from collections import List
from typing import Tuple


#a(a,b) = 1 or 0

class condition:
    def __init__(self, index, val):
        self.index : Tuple = index
        self.val  : int = val 

class Rule:
    def __init__(self, id, cond, label):
        self.rule_id : int = id
        self.conditions:  List[condition] = cond 
        self.label : int = label 
        
class Instance:
    def __init__(self, cond):
        self.conditions : List[condition] =  cond 



def main():
    cons =  [condition(2,0), condition(4,1),condition(7,1),condition(8,0)]
    Rule r = [Rule(0, cons, 1)]


if __name__ == "__main__":
    main()


