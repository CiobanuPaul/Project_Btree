#include <fstream>
#include <vector>
#include <tuple>

std::ifstream fin("abce.in");
std::ofstream fout("abce.out");

struct Node{
    //keys[i] is between children[i] and children[i+1]
    std::vector<int> keys;
    std::vector<Node*> children;
    Node* parent;

    //returns the position of the key, if found, or the position of the child to continue searching in
    int keyBinarySearch(const int x, const int l=0, int r=-1) const{
        if(r == -1){
            r = keys.size()-1;
        }
        int m = (l+r)/2;
        if(x == keys[m]) return m;
        else if(x < keys[m]){
            if(m == 0 || keys[m-1] < x)
                return m;
            return keyBinarySearch(x, l, m-1);
        }
        else if(keys[m] < x){
            if(m == keys.size()-1 || x < keys[m+1])
                return m+1;
            return keyBinarySearch(x, m+1, r);
        }
        throw; //if it got until here, there is a problem
    }
};

class Btree{
    const int t;
    int n;
    Node* root;
    //int height;

public:
    Btree(int t) : t(t){
        root = nullptr;
        n = 0;

    }

    void deleteNodes(Node* node){
        for(auto it: node->children)
            if(it != nullptr)
                deleteNodes(it);
        delete node;
    }

    ~Btree(){
        deleteNodes(root);
    }

    //returns the node and the position of the key
    std::tuple<Node*, int> searchTree(const int x, Node* node = nullptr) const{
        if(node == nullptr){
            node = root;
            if(node == nullptr) //if the tree is empty
                throw;
        }
        int p = node->keyBinarySearch(x);
        if(p != node->keys.size() && node->keys[p] == x)
            return std::make_tuple(node, p);
        if(node->children[p] != nullptr)
            return searchTree(x, node->children[p]);
        return std::make_tuple(node, p);  //to know where it stopped
    }

    bool isFound(int x) const{
        auto tup = searchTree(x);
        Node* node = std::get<0>(tup);
        int p = std::get<1>(tup);
        if(p < node->keys.size() && node->keys[p] == x)
            return true;
        return false;
    }

    int lower(const int x, Node* node= nullptr) const {
        if(node == nullptr)
            node= root;
        int i = 0, guess;//guess will have the current key, which is smaller than x, but we don't know if it is the highest
        while(i<node->keys.size() && node->keys[i] <= x)
            i++;
        if(i > 0) {
            guess = node->keys[i - 1];
            if(guess == x)
                return x;
        }
        if(node->children[i] != nullptr) { //if node not a leaf
            int guess2 = lower(x, node->children[i]);    //continuing the search on the right branch of guess1
            if (i == 0)//we don't have guess1
                return guess2;
            else{
                if(guess2 > x)
                    return guess;
                else
                    return guess2;
            }
        }
        return i>0 ? node->keys[i-1] : node->keys[i];
    }

    int higher(const int x, Node* node = nullptr){
        if(node == nullptr)
            node= root;
        int i = 0, guess;//guess will have the current key, which is higher than x, but we don't know if it is the smallest
        while(i<node->keys.size() && node->keys[i] < x)
            i++;
        if(i < node->keys.size()) {
            guess = node->keys[i];
            if(guess == x)
                return x;
        }
        if(node->children[i] != nullptr) { //if node not a leaf
            int guess2 = higher(x, node->children[i]);    //continuing the search on the left branch of guess1
            if (i == node->keys.size())//we don't have guess1
                return guess2;
            else{
                if(guess2 < x)
                    return guess;
                else
                    return guess2;
            }
        }
        return i < node->keys.size() ? node->keys[i] : node->keys[i - 1];
    }

    void between(int x, int y, bool printAll = false, Node* node = nullptr){
        if(node == nullptr){
            node = root;
        }

        if(printAll){
            int i;
            if(node->children[0] != nullptr) {
                for (i = 0; i < node->keys.size(); i++) {
                    between(x, y, true, node->children[i]);
                    fout << node->keys[i] << ' ';
                }
                between(x, y, true, node->children[i]);

            }
            else
                for(i=0; i<node->keys.size(); i++)
                    fout << node->keys[i] << ' ';
        }
        else{
            int p = node->keyBinarySearch(x);
            int pos = p;
            if(pos < node->keys.size() && node->keys[pos] == x) {
                fout << x << " ";
                pos++;
            }
            if(node->children[0] != nullptr) {
                while (pos < node->keys.size() && node->keys[pos] <= y) {
                    if (pos < node->keys.size() && node->keys[pos] <= y && p != pos)
                        printAll = true;
                    between(x, y, printAll, node->children[pos]);
                    fout << node->keys[pos] << " ";
                    pos++;
                }
                if(pos == 0)
                    between(x, y, false, node->children.front());
                else if (node->keys[pos-1] < y)//aka we keep searching in the right child where we stopped
                    between(x, y, false, node->children[pos]);
            }
            else
                while (pos < node->keys.size() && node->keys[pos] <= y) {
                    fout << node->keys[pos] << " ";
                    pos++;
                }

        }
    }

    void divideNode(Node* node){
        Node* node1 = new Node;
        Node* node2 = new Node;
        uint m = (node->keys.size() - 1)/2;

        node1->keys.insert(node1->keys.end(), node->keys.begin(), node->keys.begin() + m);//the middle element will be inserted in the parent
        node1->children.insert(node1->children.end(), node->children.begin(), node->children.begin() + m + 1);
        for(auto it: node1->children) {
            if(it == nullptr)
                break;
            it->parent = node1;
        }

        node2->keys.insert(node2->keys.end(), node->keys.begin() + m + 1, node->keys.end());
        node2->children.insert(node2->children.end(), node->children.begin() + m + 1, node->children.end());
        for(auto it: node2->children) {
            if(it == nullptr)
                break;
            it->parent = node2;
        }

        int newX = node->keys[m];

        if(node == root){
            Node* newRoot = new Node;
            newRoot->keys.push_back(newX);
            newRoot->children.push_back(node1);
            newRoot->children.push_back(node2);
            newRoot->parent = nullptr;
            node1->parent = newRoot;
            node2->parent = newRoot;
            root = newRoot;
            delete node;
            return;
        }

        Node* parent_ = node->parent;
        node1->parent = parent_;
        node2->parent = parent_;

        int newM = parent_->keyBinarySearch(newX);
        parent_->keys.insert(parent_->keys.begin()+newM, newX);
        parent_->children.insert(parent_->children.begin() + newM, node1);
        parent_->children[newM + 1] = node2;
        delete node;

        if(parent_->keys.size() == 2*t)
            divideNode(parent_);
    }

    void insertKey(const int x){
        if(root == nullptr){
            root = new Node;
            root->parent = nullptr;
            root->children.push_back(nullptr);
            root->keys.push_back(x);
            root->children.push_back(nullptr);
            n++;
            return;
        }

        auto tup = searchTree(x);
        Node* node = std::get<0>(tup);
        int p = std::get<1>(tup);
        node->keys.insert(node->keys.begin() + p, x);
        node->children.push_back(nullptr);  //all children are nullptr because it's a leaf
        n++;
        if(node->keys.size() == 2*t)
            divideNode(node);

    }

                                //x is from deletion
    void repairNode(Node* node, int x) {
        if(node == root){ //if root is empty, delete it and make a new root
            root = root->children[0]; //it's the only child
            root->parent = nullptr;
            delete node;
            return;
        }
        int pos = node->parent->keyBinarySearch(x); //the position of the parent

        if(node->parent->children[pos] != node)//just like in function delete, the binary search won't work correctly because the tree is biased
            pos--;

        if (pos > 0 && node->parent->children[pos - 1]->keys.size() >t - 1) {  //checking if the left sibling has keys to borrow
            Node* lsibl = node->parent->children[pos-1];
            //inserting the parent key
            int parentKey = node->parent->keys[pos-1];
            node->keys.insert(node->keys.begin(), parentKey);
            //overwriting the parent key with the sibling key
            node->parent->keys[pos-1] = lsibl->keys.back();
            lsibl->keys.pop_back();
            //inserting the last child from the sibling
            node->children.insert(node->children.begin(), lsibl->children.back());
            lsibl->children.pop_back();
            node->children.front()->parent = node;
        }
        else if(pos < node->parent->keys.size() && node->parent->children[pos+1]->keys.size() > t-1) {//checking if the right sibling has keys to borrow
            Node* rsibl = node->parent->children[pos+1];
            //inserting the parent key
            int parentKey = node->parent->keys[pos];
            node->keys.insert(node->keys.end(), parentKey);
            //now the parent key is overwritten by the sibling's first key
            node->parent->keys[pos] = rsibl->keys.front();
            rsibl->keys.erase(rsibl->keys.begin());
            //inserting the last child from the sibling
            node->children.insert(node->children.end(), rsibl->children.front());
            rsibl->children.erase(rsibl->children.begin());
            node->children.back()->parent = node;
        }
        else if(pos>0){//merge the node with the lsibl, and the parent key
            Node* lsibl = node->parent->children[pos-1];
            //first, the parent key is inserted at the end of the lsibl, and x is erased
            int parentKey = node->parent->keys[pos-1];
            lsibl->keys.insert(lsibl->keys.end(), parentKey);
            //merge the nodes
            lsibl->keys.insert(lsibl->keys.end(), node->keys.begin(), node->keys.end());
            lsibl->children.insert(lsibl->children.end(), node->children.begin(), node->children.end());
            //repairing the parent pointers from the children
            for(auto it: node->children)
                it->parent = lsibl;
            //delete what we extracted from the parent and delete this node
            node->parent->keys.erase(node->parent->keys.begin() + pos - 1);
            node->parent->children.erase(node->parent->children.begin() + pos);
            delete node;
            if(lsibl->parent->keys.size() < t-1 && lsibl->parent != root || lsibl->parent == root && root->keys.empty())
                repairNode(lsibl->parent, x);
        }
        else if(pos == 0){
            Node* rsibl = node->parent->children[pos+1];
            //first, the parent key is inserted at the end the node, and x is erased
            int parentKey = node->parent->keys[pos];
            node->keys.insert(node->keys.end(), parentKey);
            //merge the nodes
            node->keys.insert(node->keys.end(), rsibl->keys.begin(), rsibl->keys.end());
            node->children.insert(node->children.end(), rsibl->children.begin(), rsibl->children.end());
            //repairing the parent pointers from the children
            for(auto it: rsibl->children)
                it->parent = node;
            //delete what we extracted from the parent and delete rsibl
            rsibl->parent->keys.erase(rsibl->parent->keys.begin() + pos);
            rsibl->parent->children.erase(rsibl->parent->children.begin() + pos + 1);
            delete rsibl;
            if(node->parent->keys.size() < t-1 && node->parent != root  || node->parent == root && root->keys.empty())
                repairNode(node->parent, x);
        }
    }

    void deleteKey(const int x, Node* start= nullptr){
        auto tup = searchTree(x, start);
        Node* node = std::get<0>(tup);
        int p = std::get<1>(tup);
        if(p == node->keys.size() || x != node->keys[p])   //if it doesn't exist
            return;
        if(node->children[0] == nullptr) {//if it is a leaf
            n--;
            if(node->keys.size() > t-1 || node == root){
                node->keys.erase(node->keys.begin() + p);
                node->children.pop_back();
                return;
            }
            int pos;
            pos = node->parent->keyBinarySearch(x); //the position of the parent
            if(node->parent->children[pos] != node && node->parent->children[pos-1] == node){//if there is an inversion of parent key and predecesor(see if not a leaf case) the binary search won't always work perfectly
                pos--;
            }

            if(pos > 0 && node->parent->children[pos-1]->keys.size() > t-1) {  //checking if the left sibling has keys to borrow
                Node* lsibl = node->parent->children[pos-1];
                //first, the parent key is inserted at the beginning of the node, and x is erased
                node->keys.erase(node->keys.begin()+p);
                int parentKey = node->parent->keys[pos-1];
                node->keys.insert(node->keys.begin(), parentKey);
                //now the parent key is overwritten by the sibling's largest key
                node->parent->keys[pos-1] = lsibl->keys.back();
                //now we delete the sibling key
                lsibl->keys.pop_back();
                lsibl->children.pop_back();
            }
            else if(pos < node->parent->keys.size() && node->parent->children[pos+1]->keys.size() > t-1) {//checking if the right sibling has keys to borrow
                Node* rsibl = node->parent->children[pos+1];
                //first, the parent key is inserted at the end the node, and x is erased
                node->keys.erase(node->keys.begin()+p);
                int parentKey = node->parent->keys[pos];
                node->keys.insert(node->keys.end(), parentKey);
                //now the parent key is overwritten by the sibling's first key
                node->parent->keys[pos] = rsibl->keys.front();
                //now we delete the sibling key
                rsibl->keys.erase(rsibl->keys.begin());
                rsibl->children.pop_back();
            }
            else if(pos>0){//merge the node with the lsibl
                Node* lsibl = node->parent->children[pos-1];
                //first, the parent key is inserted at the end of the lsibl, and x is erased
                node->keys.erase(node->keys.begin()+p);
                int parentKey = node->parent->keys[pos-1];
                lsibl->keys.insert(lsibl->keys.end(), parentKey);
                //merge the nodes
                lsibl->keys.insert(lsibl->keys.end(), node->keys.begin(), node->keys.end());
                lsibl->children.insert(lsibl->children.end(), node->children.begin(), node->children.end());
                lsibl->children.pop_back();
                node->parent->keys.erase(node->parent->keys.begin() + pos - 1);
                node->parent->children.erase(node->parent->children.begin() + pos);
                delete node;
                if((lsibl->parent->keys.size() < t-1 && lsibl->parent != root) || (lsibl->parent == root && root->keys.empty()))
                    repairNode(lsibl->parent, x);
            }
            else if(pos == 0){
                Node* rsibl = node->parent->children[pos+1];
                //first, the parent key is inserted at the end the node, and x is erased
                node->keys.erase(node->keys.begin()+p);
                int parentKey = node->parent->keys[pos];
                node->keys.insert(node->keys.end(), parentKey);
                //merge the nodes
                node->keys.insert(node->keys.end(), rsibl->keys.begin(), rsibl->keys.end());
                node->children.insert(node->children.end(), rsibl->children.begin(), rsibl->children.end());
                node->children.pop_back();
                rsibl->parent->keys.erase(rsibl->parent->keys.begin() + pos);
                rsibl->parent->children.erase(rsibl->parent->children.begin() + pos + 1);
                delete rsibl;
                if(node->parent->keys.size() < t-1 && node->parent != root || node->parent == root && root->keys.empty())
                    repairNode(node->parent, x);
            }
        }
        else{ //if it is not a leaf
            //searching for the predecesor of x to interchange it with x, and then delete x as a leaf
            Node *aux = node->children[p];
            while(aux->children.back() != nullptr)
                aux = aux->children.back();
            int pred = aux->keys.back();
            aux->keys.back() = x;
            node->keys[p] = pred;
            deleteKey(x, aux);
        }
    }

    void printTree(Node* node= nullptr, int h= 0) const{
        if(node == nullptr) {
            node = root;
        }

        fout << "LEVEL " << h << "\n";
        fout<<"Node: ";
        for(auto it: node->keys)
            fout<<it<<" ";
        fout<<"\n";

        for(auto it: node->children) {
            if (it == nullptr)
                break;
            printTree(it, h+1);
        }
    }
};

int main() {
    Btree tree(2);
    int q;
    fin>>q;
    for(int i=0; i<q; i++){
        int opt;
        int x;
        fin>>opt;
        fin>>x;
        switch(opt){
            case 1:
                tree.insertKey(x);
                break;
            case 2:
                tree.deleteKey(x);
                break;
            case 3:
                fout<<tree.isFound(x)<<"\n";
                break;
            case 4:
                fout<<tree.lower(x)<<"\n";
                break;
            case 5:
                fout<<tree.higher(x)<<"\n";
                break;
            case 6:
                int y;
                fin>>y;
                tree.between(x, y);
                fout<<"\n";
                break;
        }
    }
    fin.close();
    fout.close();
    return 0;
}