#include<fstream>
#include <iostream>
#include <sstream>
#include <string>
#include<unordered_map>
#include<vector>
#include <algorithm>
#include<cmath>
using namespace std;

struct advertisement_rate{
	int advertisement_id;
	float rating;
};

bool compare_advertisement (const advertisement_rate& lhs,const advertisement_rate& rhs){
	return lhs.rating>rhs.rating;
}

unordered_map<int, unordered_map<int, float>> rating_map;
unordered_map<int, vector<int>> advertisement_user;
unordered_map<int, float> avg_rating;
unordered_map<int, string> advertisement_map;

unordered_map<int, vector<advertisement_rate>> cache;

bool read_advertisementtitle(string filename) {
	fstream advertisement_title_file(filename, ios::in);
	if (!advertisement_title_file.is_open())
		return false;

	int id_int;
	string ID, year, title;
	while (getline(advertisement_title_file, ID, ',')) {
		getline(advertisement_title_file, year, ',');
		getline(advertisement_title_file, title);

		id_int = stoi(ID);
		//cout<<id_int<<endl;
		advertisement_map[id_int] = title;
	}

	advertisement_title_file.close();
	return true;
}

bool read_rating(string filename) {
	fstream rating_file(filename, ios::in);
		if (!rating_file.is_open())
			return false;

		int user_id, advertisement_id,rating;
		string advertisement_id_str, user_id_str, rating_str;
		while (getline(rating_file, advertisement_id_str, ' ')) {
			getline(rating_file, user_id_str, ' ');
			getline(rating_file, rating_str);
			user_id = stoi(user_id_str);
			advertisement_id = stoi(advertisement_id_str);
			rating=stof(rating_str);

			/*if(rating_map.find (user_id) == rating_map.end()){
        	  unordered_map<int, float> temp;
        	  rating_map[user_id]=temp;
			}*/

          rating_map[user_id][advertisement_id]=rating;

          /*if (advertisement_user.find (advertisement_id) == advertisement_user.end()){
        	  vector<int> temp;
        	  advertisement_user[advertisement_id]=temp;
          }*/

          advertisement_user[advertisement_id].push_back(user_id);
	}

	rating_file.close();
	return true;
}

void user_rating_avg(){
	for (auto& user_it : rating_map){
		float sum=0;
		int count = 0;
		for (auto& user_advertisement_it : user_it.second){
			sum +=user_advertisement_it.second;
			++count;
		}
		avg_rating[user_it.first]=sum/count;
	}
}

unordered_map<int, float> get_correlation(int active_user){
	unordered_map<int, float> corr_map;
	unordered_map<int, float>& active_user_advertisement_rate=rating_map[active_user];

	for(auto& user_it: rating_map){
		int user_id=user_it.first;

		if(user_id==active_user)
			continue;

		unordered_map<int, float>& user_advertisement_rate=user_it.second;

		float nominator = 0;
		float sum_vaj_diff = 0;
		float sum_vij_diff = 0;

		for(auto& advertisement_it: active_user_advertisement_rate){
			int advertisement_id = advertisement_it.first;

			if (user_advertisement_rate.find(advertisement_id) == user_advertisement_rate.end())
				 continue;

			nominator +=  (rating_map[active_user][advertisement_id] - avg_rating[active_user]) * (rating_map[user_id][advertisement_id] - avg_rating[user_id]);
			sum_vaj_diff +=  pow(rating_map[active_user][advertisement_id] - avg_rating[active_user], 2);
			sum_vij_diff +=  pow(rating_map[user_id][advertisement_id] - avg_rating[user_id], 2);

		}

        float denominator= sqrt(sum_vaj_diff * sum_vij_diff);
        if(denominator!=0)
        	corr_map[user_id] = nominator/denominator;
	}

	return corr_map;
}


void advertisement_recommendation(int active_user, int K){
	vector<advertisement_rate>* predicted_rating=NULL;

	if(cache.find(active_user)==cache.end()){
		unordered_map<int, float> corr_active = get_correlation(active_user);
		predicted_rating=new vector<advertisement_rate>();

		for(auto& advertisement_it : advertisement_map){
			float pred_rating = 0;
			int advertisement_id=advertisement_it.first;

			if(advertisement_user.find(advertisement_id)!=advertisement_user.end()){

				vector<int>& user_list=advertisement_user[advertisement_id];

				for(int user_id : user_list){
					if (corr_active.find(user_id) == corr_active.end())
						continue;

					pred_rating +=corr_active[user_id]*(rating_map[user_id][advertisement_id]- avg_rating[user_id]);
				}
			}

			advertisement_rate m;
			m.advertisement_id=advertisement_id;
			m.rating=pred_rating;

			predicted_rating->push_back(m);

		}
		sort(predicted_rating->begin(), predicted_rating->end(),compare_advertisement);

		cache[active_user]=*predicted_rating;
		corr_active.clear();
	}else{
		predicted_rating=&cache[active_user];
	}
	string filename = to_string(active_user);
	ofstream ot(filename.c_str());
	cout<<"\n"<<K <<" recommended ads for user : "<<active_user<<"\n";
	for(int i=0;i<K;++i){
		//ot.open(filename.c_str());
		ot<<active_user<<" "<<advertisement_map[predicted_rating->at(i).advertisement_id]<<endl;
		cout<<advertisement_map[predicted_rating->at(i).advertisement_id]<<"\n";}

}

void clear_all(){
	avg_rating.clear();
	advertisement_map.clear();

	for(auto& it:rating_map)
		it.second.clear();

	rating_map.clear();

	for(auto& it:advertisement_user)
		it.second.clear();

	advertisement_user.clear();

	for(auto& it:cache)
		it.second.clear();

	cache.clear();
}

int main(int argc, char ** argv) {

	string advertisement_title_file="/home/keyrooh/Desktop/FYP/Review2/rec/movie_titles.txt";
	string rating_file="/home/keyrooh/Desktop/FYP/Review2/rec/ratings.txt";


	cout<< "Reading file \""<<advertisement_title_file<<"\" ...\n";
	if(read_advertisementtitle(advertisement_title_file)==false){
		cout << advertisement_title_file<<" not found!\n";
		return 1;
	}

	cout<< "Reading file \""<<rating_file<<"\" ...\n";
	if(read_rating(rating_file)==false){
		cout << rating_file<<" not found!\n";
		return 1;
	}

	cout<< "Computing User average rating.... \n";
	user_rating_avg();
	int n;
	cin>>n;
	int user_id, K;
	/*while(true){
		cout<<"Please Enter UserID (or enter -1 to exit): ";
		cin>>user_id;

		if(user_id==-1){
			clear_all();
			break;
		}

		if(rating_map.find(user_id)==rating_map.end())
			cout<<"UserID not found, Please enter valid UserID\n";
		else{
			cout<<"Please Enter Number of Recommendation: ";
			cin>>K;

			if(K>0)
				advertisement_recommendation(user_id, K);
			else
				cout<<"Number of recommendation must be greater than 0\n";

		}

		cout<<"\n";
	}*/
	while(n--){
	cout<<"USER ID :";
	cin>>user_id;
   	cout<<"Recommendations?";
	cin>>K;
	advertisement_recommendation(user_id, K);
	cout<<endl;
	}
	return 0;
}
