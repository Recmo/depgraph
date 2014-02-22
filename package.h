#pragma once
#include "utilities.h"
#include "cluster.h"
#include "version.h"

class Package
{
	public:
		// typedef shared_ptr<Package> Ptr;
		typedef Package* Ptr;
		Package();
		~Package();
		
		// Properties
		uint64 id() const { return _id; }
		void set_id(uint64 value) { _id = value; }
		
		string label() const { return _label; }
		void set_label(const string& value) { _label = value; } 
		
		Cluster::Ptr cluster() const { return _cluster; }
		void set_cluster(Cluster::Ptr value) { _cluster = value; }
		
		Version::Ptr version(int t) const { return _versions[t]; }
		void set_version(int t, Version::Ptr value) { _versions[t] = value; }
		
		bool exists();
		void set_exists(bool value);
		
		// Algorithms
		void link_through();
		
		// (De)Serializer
		void to_bip(char*& cursor);
		void from_bip(char*& cursor);
		
	protected:
		uint64 _id;
		string _label;
		Cluster::Ptr _cluster;
		vector<Version::Ptr> _versions;
};

ostream& operator<<(ostream& out, const Package& in);
