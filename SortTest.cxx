#include <vector>
#include <map>
//#include <pair>
#include <algorithm>
#include <iostream>
#include <TLorentzVector.h>
using namespace std;

typedef pair<TLorentzVector,double> TlvPt;

struct TlvPtCmp {
  bool operator()( TlvPt const & a, TlvPt const & b) { 
    return a.second < b.second ;
  }
};

struct pred {
  bool operator()(TLorentzVector const & a, TLorentzVector const & b) const {
    return a.Pt() < b.Pt();
  }
};

int SortTest(){
  vector<TLorentzVector> v;
  v.push_back( TLorentzVector( 1,1,1,1 ));
  v.push_back( TLorentzVector( 3,3,3,3 ));
  v.push_back( TLorentzVector( 2,2,2,2 ));

  // cout << v.at(0).Pt() << endl;
  // cout << v.at(1).Pt() << endl;
  // cout << v.at(2).Pt() << endl;
  // std::sort(v.begin(), v.end(),  pred() );

  // cout << endl;

  // cout << v.at(0).Pt() << endl;
  // cout << v.at(1).Pt() << endl;
  // cout << v.at(2).Pt() << endl;

  // cout << endl;

  vector< TlvPt > pv;
  pv.push_back( TlvPt (v.at(0), v.at(0).Pt() ) );
  pv.push_back( TlvPt (v.at(1), v.at(1).Pt() ) );
  pv.push_back( TlvPt (v.at(2), v.at(2).Pt() ) );

  cout << pv.at(0).first.Pt() << endl;
  cout << pv.at(1).first.Pt() << endl;
  cout << pv.at(2).first.Pt() << endl;
  sort(pv.begin(), pv.end(),  TlvPtCmp() );
  cout << endl;
  cout << pv.at(0).first.Pt() << endl;
  cout << pv.at(1).first.Pt() << endl;
  cout << pv.at(2).first.Pt() << endl;

  return 0;
}
