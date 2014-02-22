#include "version.h"
#include "package.h"
#include "varint.h"
#include "dataset.h"
#include <algorithm>

Version::Version()
: _exists(false),
  _label(""),
  _fan_in_status(not_calculated),
  _fan_out_status(not_calculated)
{
}

Version::~Version()
{
}

void Version::add_dep(Ptr version)
{
	for(vector<Ptr>::iterator d=_deps.begin(); d != _deps.end(); ++d)
	{
		if(*d == version) return;
	}
	_deps.push_back(version);
	version->_revdeps.push_back(this);
}

void Version::remove_dep(Ptr version)
{
	for(vector<Ptr>::iterator d=_deps.begin(); d != _deps.end(); ++d)
	{
		if(*d == version)
		{
			_deps.erase(d);
			break;
		}
	}
	for(vector<Ptr>::iterator r=version->_revdeps.begin(); r != version->_revdeps.end(); ++r)
	{
		if(*r == this)
		{
			version->_revdeps.erase(r);
			break;
		}
	}
}

void Version::link_through()
{
	for(vector<Ptr>::iterator r=_revdeps.begin(); r !=_revdeps.end(); ++r)
	{
		Version::Ptr rev = *r;
		for(vector<Ptr>::iterator d=_deps.begin(); d != _deps.end(); ++d)
		{
			(*r)->add_dep(*d);
		}
		for(vector<Ptr>::iterator d = rev->_deps.begin(); d != rev->_deps.end(); ++d)
		{
			if(*d == this)
			{
				rev->_deps.erase(d);
				break;
			}
		}
	}
	_revdeps.clear();
	
	for(vector<Ptr>::iterator d = _deps.begin(); d != _deps.end(); ++d)
	{
		Version::Ptr dep = *d;
		for(vector<Ptr>::iterator r=dep->_revdeps.begin(); r !=dep->_revdeps.end();)
		{
			if(*r == this)
			{
				r = dep->_revdeps.erase(r);
				break;
			}
			else
			{
				++r;
			}
		}
	}
	_deps.clear();
}

void Version::reset_fan_in()
{
	_fan_in.clear();
	_fan_in_status = not_calculated;
}

void Version::calculate_fan_in()
{
	if(_fan_in_status == calculated) return;
	if(_fan_in_status == calculating)
	{
		// Cyclic
		return;
	}
	_fan_in_status = calculating;
	
	_fan_in.clear();
	_fan_in.push_back(this);
	for(vector<Ptr>::iterator r=_revdeps.begin(); r !=_revdeps.end(); ++r)
	{
		Ptr in = *r;
		_fan_in.push_back(in);
		in->calculate_fan_in();
		for(vector<Ptr>::iterator fii = in->_fan_in.begin(); fii != in->_fan_in.end(); ++fii)
		{
			if(!contains(_fan_in, *fii)) _fan_in.push_back(*fii);
		}
	}
	_fan_in_status = calculated;
}

void Version::reset_fan_out()
{
	_fan_out.clear();
	_fan_out_status = not_calculated;
}

void Version::calculate_fan_out()
{
	if(_fan_out_status == calculated) return;
	if(_fan_out_status == calculating)
	{
		// Cyclic
		return;
	}
	_fan_out_status = calculating;
	
	_fan_out.clear();
	_fan_out.push_back(this);
	for(vector<Ptr>::iterator d = _deps.begin(); d !=_deps.end(); ++d)
	{
		Ptr out = *d;
		_fan_out.push_back(out);
		out->calculate_fan_out();
		for(vector<Ptr>::iterator foi = out->_fan_out.begin(); foi != out->_fan_out.end(); ++foi)
		{
			if(!contains(_fan_out, *foi)) _fan_out.push_back(*foi);
		}
	}
	_fan_out_status = calculated;
}

void Version::from_bip(char*& cursor)
{
	set_label(read_string(cursor));
	if(!label().empty()) set_exists();
	int num_deps = read_uint(cursor);
	_deps.clear();
	_deps.reserve(num_deps);
	for(int i=0; i < num_deps; i++)
	{
		Package::Ptr pkg = dataset.package(read_uint(cursor));
		Version::Ptr dep = pkg->version(time());
		add_dep(dep);
	}
}

ostream& operator<<(ostream& out, const Version& in)
{
	out << in.package()->cluster()->label();
	out << "/" << in.package()->label();
	out << "-" << in.label();
	out << "-T" << in.time();
}



