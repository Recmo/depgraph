#include "dataset.h"
#include "varint.h"

Dataset dataset;

Cluster::Ptr Dataset::cluster(uint64 id)
{
	Cluster::Ptr cluster = _clusters[id];
	assert(cluster->id() == id);
	return cluster;
}

Cluster::Ptr Dataset::cluster(const string& label)
{
	for(int c=0; c < _clusters.size(); c++)
	{
		Cluster::Ptr cluster = _clusters[c];
		if(cluster->label() == label) return cluster;
	}
	return Cluster::Ptr();
}

Package::Ptr Dataset::package(uint64 id)
{
	Package::Ptr package = _packages[id];
	assert(package->id() == id);
	return package;
}

Package::Ptr Dataset::package(const string& fullname)
{
	string package_name = fullname;
	Cluster::Ptr cls = 0;
	int split_pos = fullname.find('/');
	if(split_pos != string::npos)
	{
		string cluster_name = fullname.substr(0, split_pos);
		string package_name = fullname.substr(split_pos + 1, fullname.length() - split_pos - 1);
		Cluster::Ptr cls = cluster(cluster_name);
	}
	for(int p=0; p < _packages.size(); p++)
	{
		if(cls && _packages[p]->cluster() != cls) continue;
		if(_packages[p]->label() == package_name)
		{
			return _packages[p];
		}
	}
	cerr << "Package " << fullname << " not found!";
	return 0;
}

//
//  E l i m i n a t i o n  p a s s e s
//

void Dataset::elliminate_virtuals()
{
	for(int p=0; p < _packages.size(); p++)
	{
		Package::Ptr pkg = package(p);
		if(pkg->cluster()->label() == "virtual-virtual")
		{
			//cerr << "P";
			cerr << "Elliminating " << *pkg << endl;
			pkg->link_through();
			pkg->set_exists(false);
			continue;
		}
		/*
		for(int t=0; t < max_t; t++)
		{
			Version::Ptr ver = pkg->version(t);
			if(!ver->exists() && (ver->in_degree() || ver->out_degree()))
			{
				// cerr << "V";
				cerr << "V Elliminating " << *ver << endl;
				ver->link_through();
				ver->set_exists(false);
			}
		}
		*/
	}
}

void Dataset::elliminate_metas()
{
	for(int p=0; p < _packages.size(); p++)
	{
		Package::Ptr pkg = package(p);
		if(ends_with(pkg->label(), "-meta"))
		{
			cerr << "M";
			cerr << "Elliminating " << *pkg << endl;
			pkg->link_through();
			pkg->set_exists(false);
		}
	}
}

void Dataset::calculate_fans()
{
	for(int t=0; t < max_t; t++)
	{
		cout << t << endl;
		for(int p=0; p < _packages.size(); p++)
		{
			Package::Ptr pkg = package(p);
			Version::Ptr ver = pkg->version(t);
			ver->calculate_fan_in();
			ver->calculate_fan_out();
		}
		for(int p=0; p < _packages.size(); p++)
		{
			Package::Ptr pkg = package(p);
			Version::Ptr ver = pkg->version(t);
			ver->reset_fan_in();
			ver->reset_fan_out();
		}
	}
}

//
//  O u t p u t   f o r m a t s
//

void Dataset::to_gml(ostream& out, int t)
{
	out << "graph [" << endl;
	out << "  directed 1" << endl;
	
	// Packages
	for(int p=0; p < _packages.size(); p++)
	{
		Version::Ptr ver = package(p)->version(t);
		if(ver->in_degree() == 0 && ver->out_degree() == 0)
		{
			continue;
		}
		out << "  node [" << endl;
		out << "    id " << p << endl;
		out << "    label \"" << *package(p) << "\"" << endl;
		out << "  ]" << endl;
	}
	
	// Edges
	for(int p=0; p < _packages.size(); p++)
	{
		Version::Ptr ver = package(p)->version(t);
		if(ver->in_degree() == 0 && ver->out_degree() == 0)
		{
			continue;
		}
		const vector<Version::Ptr>& deps = package(p)->version(t)->deps();
		for(int d=0; d < deps.size(); d++)
		{
			out << "  edge [" << endl;
			out << "    source " << p << endl;
			out << "    target " << deps[d]->package()->id() << endl;
			out << "  ]" << endl;
		}
	}
	out << "]" << endl;
}

void Dataset::to_bip(char*& cursor)
{
	
}

void Dataset::from_bip(char*& cursor)
{
	uint64 num_clusters = read_uint(cursor);
	_clusters.reserve(num_clusters);
	for(int c=0; c < num_clusters; c++)
	{
		Cluster::Ptr cluster(new Cluster);
		cluster->set_id(c);
		cluster->from_bip(cursor);
		_clusters.push_back(cluster);
	}
	
	uint64 num_packages = read_uint(cursor);
	_packages.reserve(num_packages);
	for(int p=0; p < num_packages; p++)
	{
		Package::Ptr pkg(new Package);
		pkg->set_id(p);
		_packages.push_back(pkg);
	}
	for(int p=0; p < num_packages; p++)
	{
		_packages[p]->from_bip(cursor);
	}
}


