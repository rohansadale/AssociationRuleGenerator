#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<stdlib.h>
#include<fstream>
#include<set>
#include<algorithm>
#include<ctime>

using namespace std;

// Variable Declaration
typedef map< int, vector<int> > itemsets;
double support;
double confidence;
int option;
int frequentItemsets = 0;
int associationRulesCount;
string outputfile;
map< set<int>, int > frequency_count;
vector< vector<int> > association;
vector<int> vect;
int test1 = 0;
vector< pair< set<int>, set<int> > > output;

// Frequent Itemset and Association Rules generation function declaration
int treeProjection(int , itemsets &);
vector < pair<int, int> > getFrequency(itemsets &);
itemsets projectedDatabase(int , itemsets &);
void association_rules();
void generateRules(set<int>, set<int>, int, map< pair< set<int>, set<int> >, int> &u);
void pruneIt(itemsets &);
void writeToFile();

// Print functions declaration
void printRules(map< pair< set<int>, set<int> >, int> &u);
void printVector(vector<int>);
void printMap(itemsets);
void printSet(set<int>);
void printFrequencyCount();
void printAssociation(vector< vector<int> >);


int main(int argc, char *argv[] )
{
	
	if(argc < 2)
	{
		cout<< "Usage: " << argv[0] << " minsup minconf inputfile outputfile options \n";
		return 1;	
	}	
	string inputfile;
	support		= atof(argv[1]);
	confidence 	= atof(argv[2]);
	inputfile	= argv[3];
	outputfile	= argv[4];
	option		= atof(argv[5]);
	
	map<int, vector<int> > t;
	int t_id, t_item;
	ifstream file;
	file.open(inputfile);
	while (file >> t_id){
		file >> t_item;
		t[t_id].push_back(t_item);
	}
	file.close();
	
	int start_s = clock();
	treeProjection(0,t);
	int stop_s = clock();
	double frequentItemsTime =double(stop_s-start_s)/(CLOCKS_PER_SEC);
	
	start_s = clock();
	if(support > 20)
		association_rules();
	stop_s = clock();
	double associationRulesTime = double(stop_s-start_s)/(CLOCKS_PER_SEC);
	
	ofstream myfile;
	myfile.open("Readings.csv", ios_base::app);
	myfile << support << "|" << confidence << "|" << option << "|" << frequentItemsTime << "|" << associationRulesTime << "|" << frequentItemsets << "|" << output.size() <<endl;
	myfile.close();	

	writeToFile();
	return 0;
}

int treeProjection(int item, itemsets &t){

	if(t.size() > 0){
	        vector< pair<int, int> > frequency = getFrequency(t);
                for(int i = 0; i < frequency.size() ; i++){
				vect.push_back(frequency[i].second);
	                        set<int> s(vect.begin(), vect.end());
	              	        frequency_count.insert(pair< set<int>, int >(s, frequency[i].first));

				frequentItemsets++;	               
				itemsets temp_projected_db = projectedDatabase(frequency[i].second,t); 
				treeProjection(frequency[i].second,temp_projected_db);
				vect.pop_back();
		}		
		return 0;
	}
	else{
		return 0;
	}
}

vector < pair<int, int> > getFrequency(itemsets &t){	
	itemsets::iterator it_t;
	map<int, int> freq_map;
	vector<pair<int, int>> freq_sort;
	for(it_t = t.begin(); it_t != t.end(); it_t++)
		for(int i = 0; i<it_t->second.size(); i++)
			freq_map[it_t->second[i]]++;

	map<int, int>::iterator it_m;
	for(it_m = freq_map.begin(); it_m != freq_map.end(); it_m++){
		if(it_m->second > support)
			freq_sort.push_back(make_pair(it_m->second, it_m->first));		
	}
	
	if(option == 2)
		sort(freq_sort.begin(), freq_sort.end());
	else if (option == 3)
		sort(freq_sort.rbegin(), freq_sort.rend()); 

	return freq_sort;
}

itemsets projectedDatabase(int item, itemsets &t){
	
	itemsets temp_m;	
	vector<int> temp_v;
	map< int, vector<int> >::iterator it_t;
	vector<int>::iterator it_v;
	it_t = t.begin();
	while(it_t != t.end()){
		it_v = lower_bound(it_t->second.begin(), it_t->second.end(), item);
		if(it_v != it_t->second.end()  && *it_v == item){
			
				temp_v.insert(temp_v.begin(), it_v+1, it_t->second.end());
				temp_v.insert(temp_v.begin(), it_t->second.begin(), it_v);

			        if(temp_v.size() != 0)
                                temp_m.insert(pair<int, vector<int> >(it_t->first, temp_v));
	                        temp_v.clear();
				
				it_t->second.erase(it_v);

		}
		it_t++;
	}

	return temp_m;
}

void association_rules(){
	
	vector< vector<int> >::iterator it_a;
	vector<int>::iterator it_v;
	associationRulesCount = 0;
	
	for(auto it_a = frequency_count.begin(); it_a != frequency_count.end(); it_a++){
			if(it_a->first.size() > 1) {
				set<int> full_set(it_a->first.begin(), it_a->first.end());
				set<int> empty;
				map< pair< set<int>, set<int> >, int> uniqueRules;
				generateRules(full_set, empty, frequency_count[full_set], uniqueRules);
				uniqueRules.clear();
			}
	}

}

void generateRules(set<int> left, set<int> right, int setFrequency, map< pair< set<int>, set<int> >, int> &uniqueRules){
	set<int> full_set = left;
	set<int>::iterator it_sl = full_set.end();
	int item;

	while(it_sl != full_set.begin()){
		item = *(--it_sl);
		right.insert(item);
		left.erase(item);
		if(left.size() < 1) break;
		uniqueRules[make_pair(left, right)]++;
		if (uniqueRules[make_pair(left,right)] == 1){
			if (double(setFrequency)/frequency_count[left] > confidence){
				output.push_back(make_pair(left, right));
				associationRulesCount++;
				generateRules(left, right, setFrequency, uniqueRules);
			}
		}	
		left.insert(item);;
		right.erase(item);
	}	
}

void writeToFile(){

	string temp;
	ofstream myfile(outputfile);
	if(support <= 20){
		if(myfile){				
			for(auto it_a = frequency_count.begin(); it_a != frequency_count.end(); it_a++){
				temp = "";
				for(auto it_s = it_a->first.begin(); it_s != it_a->first.end(); it_s++)
					temp = temp + to_string(*it_s) + " ";	

				temp = temp + "||" + to_string(it_a->second) + "|-1";
				myfile<<temp<<endl;  
			}
		}	
	}
	else{
		if(myfile){
			for(int i = 0; i< output.size(); i++){
				temp = "";
				for(auto it_s = output[i].first.begin(); it_s != output[i].first.end(); it_s++)
					temp = temp + to_string(*it_s) + " ";	
					
					temp = temp + "|";
				for(auto it_s = output[i].second.begin(); it_s != output[i].second.end(); it_s++)
					temp = temp + to_string(*it_s) + " ";	
			
				set<int>s(output[i].first.begin(), output[i].first.end());
				s.insert(output[i].second.begin(), output[i].second.end());
				temp = temp + "|" + to_string(frequency_count[s]) + "|" + to_string(double(frequency_count[s])/frequency_count[output[i].first]);
				myfile<<temp<<endl;
			} 
		}
	}
	myfile.close();		

}

// Print function definition below
/*
void printFrequencyCount(){
	map <set<int>, int >::iterator it_m;
	set<int>::iterator it_s;
	for(it_m = frequency_count.begin(); it_m != frequency_count.end(); it_m++){
		for(it_s = it_m->first.begin(); it_s != it_m->first.end(); it_s++)
			cout<< *it_s << " ";
		cout << "-> " << it_m-> second << endl;
	}
}
*/
void printAssociation(vector< vector<int> > v){
	vector<vector<int> >::iterator it_v = v.begin();
	while(it_v != v.end()){
//		if((*it_v).size() > 1){
			for(int i=0; i<(*it_v).size(); i++)
				cout << (*it_v)[i]<< " ";
			 cout << endl;
//		}
		it_v++;
	}		
}

void printMap(itemsets t){
	cout << "In printMap \n";
        map< int, vector<int> >::iterator it_t;
	it_t = t.begin();
        while(it_t != t.end()){
                cout<< it_t->first << ": ";
                for(int i=0 ; i<it_t->second.size() ; i++){
                        cout << it_t->second[i] << " ";
                }
                cout << endl;
                it_t++ ;
        }
	cout << "Getting out \n";
}

void printSet(set<int> items){
	set<int>::iterator it_set;
        it_set = items.begin(); 
        while(it_set != items.end()){
                cout << *it_set << " ";
                it_set++;
        }
} 

void printVector(vector<int> v){

	for(int i = 0; i < v.size() ; i++)
		cout << v[i] << " ";
	cout << endl;
}

void printRules( map< pair< set<int>, set<int> >, int> &uniqueRules){

	map< pair< set<int>, set<int> >, int>::iterator it_m;
	cout << "In here\n";

	for(it_m = uniqueRules.begin(); it_m != uniqueRules.end(); it_m++){
		printSet(it_m->first.first);	
		cout << "->";
		printSet(it_m->first.second);
		cout << " : " << it_m->second << endl;	
		//cout<< uniqueRules.size();
	}	
	cout << "Out here \n \n";
}
