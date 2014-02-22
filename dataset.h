#pragma once
#include "utilities.h"
#include "cluster.h"
#include "package.h"


class Dataset
{
	public:
		
		const vector<Cluster::Ptr>& clusters() const { return _clusters; }
		Cluster::Ptr cluster(uint64 id);
		Cluster::Ptr cluster(const string& label);
		
		const vector<Package::Ptr>& packages() const { return _packages; }
		Package::Ptr package(uint64 id);
		Package::Ptr package(const string& fullname);
		
		void elliminate_virtuals();
		void elliminate_metas();
		void calculate_fans();
		
		void to_gml(ostream& out, int time);
		
		// (De)Serializer
		void to_bip(char*& cursor);
		void from_bip(char*& cursor);
		
	protected:
		vector<Cluster::Ptr> _clusters;
		vector<Package::Ptr> _packages;
	
};

extern Dataset dataset;
