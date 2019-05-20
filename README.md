# Tema 3 - Existential Graphs Reasoner

# Goal
Our purpose was to complete the functionality of the Existential Graph
while retaining the coding style and methods of implementation
given in the skeleton of the homework. To do this we relied
on the extensive capabilities of the stl, specifically the
std::vector

# Implementation
Deiterate, double_cut, erase follow the same basic recipe. Recursively
crawling down the tree, and when the destination is reached, 
apply the operation described in the documentation of the homework.
The operations that proved to be difficult to write were possible_erasures
and possible_deiterations.

For possible_deiterations, all subgraphs of a given node are iterated,
and each one of them is searched in the other subgraphs. Same thing
is happening to the atoms. All results are offloaded in a vector,
along with results coming from recursion. At the end it is required to
remove all duplicates in the result vector, since the same path may come
from a subgraph in another subgraph.

For possible_erasures, we tried two implementations, and decided that
the recursion one is better suited to the coding style of the whole
data structure, the local stack one being noticeably longer than
all other functions, and harder to understand. // TODO scrie aici cum ai implementat-o

# Algorithms used
We mainly used the operations std::vector::erase and std::vector::insert to
keep the code minimalistic. In possible_deiterations, to remove duplicates
it was required to sort the result vector in able to apply std::unique.

## Team
### Rica Radu Leonard 315CA
Done:
* possible_double_cuts()
* double_cut(L)
* erase(L)
* possible_erasures() (recursion)

### Ciobanu Bogdan 315CA
Done:
* possible_deiterations()
* deiterate(L)
* possible_erasures() (local stack)
* README


