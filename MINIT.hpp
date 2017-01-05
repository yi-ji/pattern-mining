#include <stdio.h>
#include <vector>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include "Miner.hpp"

using namespace std;
using namespace boost;

class MINIT : public Miner
{
private:
    int max_c;
    
public:
    MINIT(TransactionDB* _db, float _ratio, int _max_c) : Miner(_db, _ratio), max_c(_max_c) {}
    virtual void mine()
    {
        Miner::mine();
        vector<bool> V(DB->dim(), true);
        MII = minit(DB, &V, max_c);
    }
    vector<Itemset>* minit(TransactionDB* D, vector<bool>* V, int max_c)
    {
        vector<Itemset>* M = new vector<Itemset>;
        if (max_c == 1)
        {
            for (map<int,int>::iterator i = D->items.begin(); i != D->items.end(); i++)
            {
                if (i->second >= threshold) {break;}
                M->push_back(Itemset(Transaction(DB->item2bt(i->first)), i->second));
            }
            return M;
        }
        for (vector<pair<int,int>>::iterator i = D->sorted_items.begin(); i != D->sorted_items.end(); i++)
        {
            if (max_c == DB->dim()) {printf("%ld/%ld %lus\n", i-D->sorted_items.begin(), D->sorted_items.size(), (clock()-miner_begin)/CLOCKS_PER_SEC);}
            if (!(*V)[DB->position(i->first)])
            {
                continue;
            }
            if (i->second == D->size())
            {
                break;
            }
            int item = i->first;
            (*V)[DB->position(item)] = false;
            if (i->second < threshold)
            {
                M->push_back(Itemset(Transaction(DB->item2bt(i->first)), i->second));
                continue; // this prunning is not in paper
            }
            
            TransactionDB* next_D = new TransactionDB(DB, D, item);
            vector<bool>* next_V = new vector<bool>(*V);
            vector<Itemset>* C = minit(next_D, next_V, max_c-1);
            delete next_D;
            delete next_V;
            
            vector<int>* supp = new vector<int>;
            for (vector<Itemset>::iterator j = C->begin(); j != C->end(); j++)
            {
                supp->push_back(threshold - j->support);
                j->support = 0;
            }
            for (vector<Transaction>::iterator j = D->db.begin(); j != D->db.end(); j++)
            {
                for (vector<Itemset>::iterator k = C->begin(); k != C->end(); k++)
                {
                    if (k->itemset.bt->is_subset_of(*(j->bt)))
                    {
                        if ((*(j->bt))[DB->position(item)] == 1)
                        {
                            k->support += 1;
                        }
                        else
                        {
                            (*supp)[k-C->begin()] -= 1;
                        }
                    }
                }
            }
            for (vector<Itemset>::iterator j = C->begin(); j != C->end(); j++)
            {
                if ((*supp)[j-C->begin()] <= 0)
                {
                    j->itemset.bt->set(DB->position(item), true);
                    M->push_back(*j);
                }
                else
                {
                    delete j->itemset.bt;
                }
            }
            delete supp;
            delete C;
        }
        return M;
    }
};