#pragma once
#include "utilities.h"

class Package;

class Version
{
	public:
		// typedef shared_ptr<Version> Ptr;
		typedef Version* Ptr;
		Version();
		~Version();
		
		bool exists() const { return _exists; }
		void set_exists(bool value = true) { _exists = value; }
		
		int time() const { return _time; }
		void set_time(int value) { _time = value; }
		
		string label() const { return _label; }
		void set_label(const string& value) { _label = value; }
		
		void add_dep(Ptr version);
		void remove_dep(Ptr version);
		
		//shared_ptr<Package> package() const { return _package; }
		// void set_package(shared_ptr<Package> value) { _package = value; }
		Package* package() const { return _package; }
		void set_package(Package* value) { _package = value; }
		
		const vector<Ptr>& deps() const { return _deps; }
		const vector<Ptr>& revdeps() const { return _revdeps; }
		
		int in_degree() const { return _revdeps.size(); }
		int out_degree() const { return _deps.size(); }
		
		// Algorithms
		void link_through();
		
		void reset_fan_in();
		void calculate_fan_in();
		int fan_in_size() const { return _fan_in.size(); }
		const vector<Ptr>& fan_in() const { return _fan_in; }
		
		void reset_fan_out();
		void calculate_fan_out();
		int fan_out_size() const { return _fan_out.size(); }
		const vector<Ptr>& fan_out() const { return _fan_in; }
		
		// Input/output
		void to_bip(char*& cursor);
		void from_bip(char*& cursor);
		
	protected:
		
		enum fan_status {
			not_calculated,
			calculating,
			calculated
		};
		
		bool _exists;
		int _time;
		string _label;
		// shared_ptr<Package> _package;
		Package* _package;
		vector<Ptr> _deps;
		vector<Ptr> _revdeps;
		vector<Ptr> _fan_in;
		vector<Ptr> _fan_out;
		fan_status _fan_in_status;
		fan_status _fan_out_status;
		
};

ostream& operator<<(ostream& out, const Version& in);
