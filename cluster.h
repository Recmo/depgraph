#pragma once
#include "utilities.h"

class Cluster
{
	public:
		// typedef shared_ptr<Cluster> Ptr;
		typedef Cluster* Ptr;
		Cluster();
		~Cluster();
		
		uint64 id() const { return _id; }
		void set_id(uint64 value) { _id = value; }
		
		const string label() const { return _label; }
		void set_label(const string& label) { _label = label; }
		
		void to_bip(char*& cursor);
		void from_bip(char*& cursor);
		
	protected:
		uint64 _id;
		string _label;
};

ostream& operator<<(ostream& out, const Cluster& in);
