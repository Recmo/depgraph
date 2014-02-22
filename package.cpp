#include "dataset.h"
#include "package.h"
#include "varint.h"

Package::Package()
: _versions(max_t)
{
	for(int t=0; t < max_t; t++)
	{
		Version::Ptr ver(new Version);
		ver->set_time(t);
		_versions[t] = ver;
	}
}

Package::~Package()
{
	
}

bool Package::exists()
{
	for(int t=0; t < max_t; t++)
	{
		if(_versions[t]->exists()) return true;
	}
	return false;
}

void Package::set_exists(bool value)
{
	assert(!value);
	for(int t=0; t < max_t; t++)
	{
		_versions[t]->set_exists(false);
	}
}

void Package::link_through()
{
	int max_out_degree = 0;
	int max_in_degree = 0;
	for(int t=0; t < max_t; t++)
	{
		max_in_degree = max(max_in_degree, version(t)->in_degree());
		max_out_degree = max(max_out_degree, version(t)->out_degree());
		version(t)->link_through();
	}
	cerr << max_out_degree  <<  "\t" << max_in_degree << "\t" << max_out_degree * max_in_degree << endl;
}

void Package::to_bip(char*& cursor)
{
}

void Package::from_bip(char*& cursor)
{
	set_cluster(dataset.cluster(read_uint(cursor)));
	set_label(read_string(cursor));
	for(int t=0; t < max_t; t++)
	{
		_versions[t]->set_package(dataset.package(id()));
		_versions[t]->from_bip(cursor);
	}
}

ostream& operator<<(ostream& out, const Package& in)
{
	out << in.cluster()->label() << "/" << in.label();
	return out;
}
