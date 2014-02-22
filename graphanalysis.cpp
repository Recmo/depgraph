#include "cluster.h"
#include "package.h"
#include "version.h"
#include "dataset.h"
#include "math.h"

/*
vector<string> clusters;
vector<Package*> packages;

void add_package(vector<Package*>& list, Package* pkg)
{
	for(int i=0; i < list.size(); i++)
	{
		if(list[i] == pkg) return;
	}
	list.push_back(pkg);
}

void remove_package(vector<Package*>& list, Package* pkg)
{
	for(int i=0; i < list.size(); i++)
	{
		if(list[i] == pkg)
		{
			list.erase(list.begin() + i);
			i--;
		}
	}
}

void remove_package(Package* pkg)
{
	remove_package(packages, pkg);
	for(int t=0; t < max_timecode; t++)
	{
		Version& ver = pkg->versions[t];
		for(int d=0; d < ver.deps.size(); d++)
		{
			remove_package(ver.deps[d]->versions[t].reverse, pkg);
		}
		for(int r=0; r < ver.reverse.size(); r++)
		{
			remove_package(ver.reverse[r]->versions[t].deps, pkg);
		}
	}
	delete pkg;
}

inline ostream& operator<<(ostream& out, const Package& in)
{
	out << clusters[in.cluster] << "/" << in.label;
	return out;
}


int package_index(Package* pkg)
{
	for(int p=0; p < packages.size(); p++)
	{
		if(packages[p] == pkg) return p;
	}
	return -1;
}

Package* read_package(char* &cursor)
{
	Package* pkg = new Package();
	pkg->cluster = read_uint(cursor);
	pkg->label = read_string(cursor);
	for(int t=0; t < max_timecode; t++)
	{
		pkg->versions[t].version = read_string(cursor);
		int num_deps = read_uint(cursor);
		// cout << num_deps << " ";
		pkg->versions[t].deps.reserve(num_deps);
		for(int i=0; i < num_deps; i++)
		{
			pkg->versions[t].deps.push_back((Package*)read_uint(cursor));
		}
	}
	return pkg;
}

void deserialize_references()
{
	for(int p=0; p < packages.size(); p++)
	for(int t=0; t < max_timecode; t++)
	{
		vector<Package*>& deps = packages[p]->versions[t].deps;
		for(int d=0; d < deps.size(); d++)
		{
			deps[d] = packages[(uint64)(deps[d])];
		}
	}
}

void calculate_reverse_deps()
{
	for(int p=0; p < packages.size(); p++)
	for(int t=0; t < max_timecode; t++)
	{
		vector<Package*>& deps = packages[p]->versions[t].deps;
		for(int d=0; d < deps.size(); d++)
		{
			deps[d]->versions[t].reverse.push_back(packages[p]);
		}
	}
}

void elliminate_version(Package& pkg, int t)
{
	Version& ver = pkg.versions[t];
	
	// Connect all reverse links trough
	for(int d=0; d < ver.deps.size(); d++)
	{
		Package& dep = *ver.deps[d];
		for(int r=0; r < ver.reverse.size(); r++)
		{
			if(&dep == ver.reverse[r]) continue;
			add_package(dep.versions[t].reverse, ver.reverse[r]);
		}
		remove_package(dep.versions[t].reverse, &pkg);
	}
	
	for(int r=0; r < ver.reverse.size(); r++)
	{
		Package& rev = *ver.reverse[r];
		for(int d=0; d < ver.deps.size(); d++)
		{
			if(&rev == ver.deps[d]) continue;
			add_package(rev.versions[t].deps, ver.deps[d]);
		}
		remove_package(rev.versions[t].deps, &pkg);
	}
	
	ver.deps.clear();
	ver.reverse.clear();
}

void elliminate_package(Package &pkg)
{
	for(int t=0; t < max_timecode; t++)
	{
		Version& ver = pkg.versions[t];
		if(ver.deps.size() == 0 && ver.reverse.size() == 0) continue;
		elliminate_version(pkg, t);
	}
}

void elliminate_virtuals()
{
	// Remove virtual-virtual
	for(int p=0; p < packages.size(); p++)
	{
		Package& pkg = *packages[p];
		if(clusters[pkg.cluster] != "virtual-virtual") continue;
		
		// cerr << "Elliminating " << pkg << endl;
		elliminate_package(pkg);
		remove_package(&pkg);
		p--;
	}
	
	// Remove non-existing versions
	for(int t=0; t < max_timecode; t++)
	{
		for(int p=0; p < packages.size(); p++)
		{
			Package& pkg = *packages[p];
			Version& ver = pkg.versions[t];
			if(ver.exists()) continue;
			if(ver.deps.size() == 0 && ver.reverse.size() == 0) continue;
			elliminate_version(pkg, t);
		}
	}
	
	// Remove pure none-existing packages
	for(int p=0; p < packages.size(); p++)
	{
		Package& pkg = *packages[p];
		bool pure = true;
		for(int t=0; t < max_timecode; t++)
		{
			Version& ver = pkg.versions[t];
			if(ver.exists())
			{
				pure = false;
				break;
			}
		}
		if(!pure) continue;
		//cerr << "Removing " << pkg  << endl;
		remove_package(&pkg);
		p--;
	}
}

void elliminate_metas()
{
	vector<Package*> to_elliminate;
	for(int p=0; p < packages.size(); p++)
	{
		Package& pkg = *packages[p];
		
		if(ends_with(pkg.label, "-meta"))
		{
			cerr << "Elliminating " << pkg << endl;
			to_elliminate.push_back(&pkg);
		}
	}
	for(int p=0; p < to_elliminate.size(); p++)
	{
		elliminate_package(*to_elliminate[p]);
		remove_package(to_elliminate[p]);
	}
}

void check_consistency()
{
	// Every package must exists sometime
	for(int p=0; p < packages.size(); p++)
	{
		bool exists = false;
		for(int t=0; t < max_timecode; t++)
		{
			if(packages[p]->versions[t].exists())
			{
				exists = true;
				break;
			}
		}
		if(!exists)
		{
			cerr << "Package " << clusters[packages[p]->cluster] << 
			"/" << packages[p]->label << " does not exist." << endl;
		}
	}
	
	// Every dependency must exist in the same time
	for(int t=0; t < max_timecode; t++)
	{
		for(int p=0; p < packages.size(); p++)
		{
			Package* pkg = packages[p];
			if(!pkg->versions[t].exists()) continue;
			
			vector<Package*>& deps = packages[p]->versions[t].deps;
			for(int d=0; d < deps.size(); d++)
			{
				if(!deps[d]->versions[t].exists())
				{
					cerr << t << ": " << clusters[pkg->cluster] << "/" << pkg->label << " depends on not existing " << clusters[deps[d]->cluster] << "/" << deps[d]->label << endl;
				}
			}
		}
	}
	
	// Every dependency must be listed as a reverse once
	for(int t=0; t < max_timecode; t++)
	{
		for(int p=0; p < packages.size(); p++)
		{
			Package* pkg = packages[p];
			if(!pkg->versions[t].exists()) continue;
			
			vector<Package*>& deps = packages[p]->versions[t].deps;
			for(int d=0; d < deps.size(); d++)
			{
				int listings = 0;
				vector<Package*>& revs = deps[d]->versions[t].reverse;
				for(int r=0; r< revs.size(); r++)
				{
					if(revs[r] == pkg) listings++;
				}
				if(listings != 1)
				{
					cerr << "Package " << *pkg << " listed " << listings << " times in " << *deps[d] << endl;
				}
			}
		}
	}
}

void dump_graph_tlp(int t)
{
	cout << "(tlp \"2.0\"" << endl;
	
	// Package ids
	cout << "(nodes ";
	for(int p=0; p < packages.size(); p++)
	{
		if(!packages[p]->versions[t].exists()) continue;
		cout << p << " ";
	}
	cout << ")" << endl;
	
	// Package labels
	cout << "(property 0 string \"viewLabel\"" << endl;
	cout << "  (default \"\" \"\" )" << endl;
	for(int p=0; p < packages.size(); p++)
	{
		if(!packages[p]->versions[t].exists()) continue;
		cout << "  (node " << p << " \"" << packages[p]->label << "\")" << endl;
	}
	cout << ")" << endl;
	
	// Clusters
	for(int c=0; c < clusters.size(); c++)
	{
		cout << "(cluster " << c + 1 << " \"" << clusters[c] << "\"";
		cout << " (nodes ";
		for(int p=0; p < packages.size(); p++)
		{
			if(!packages[p]->versions[t].exists()) continue;
			if(packages[p]->cluster != c) continue;
			cout << p << " ";
		}
		cout << "))" << endl;
	}
	
	// Edges
	int edgeid = 0;
	for(int p=0; p < packages.size(); p++)
	{
		if(!packages[p]->versions[t].exists()) continue;
		vector<Package*>& deps = packages[p]->versions[t].deps;
		for(int d=0; d < deps.size(); d++)
		{
			int depp = package_index(deps[d]);
			cout << "(edge " << edgeid++ << " " << p << " " << depp << ")" << endl;
		}
	}
	
	cout << ")" << endl;
}

void dump_graph_lgl(int t)
{
	for(int p=0; p < packages.size(); p++)
	{
		if(!packages[p]->versions[t].exists()) continue;
		cout << "# " <<  p << endl;
		
		vector<Package*>& deps = packages[p]->versions[t].deps;
		for(int d=0; d < deps.size(); d++)
		{
			int depp = package_index(deps[d]);
			cout << depp << endl;
		}
	}
}

void dump_graph_gml(int t)
{
	cout << "graph [" << endl;
	cout << "  directed 1" << endl;
	
	// Packages
	for(int p=0; p < packages.size(); p++)
	{
		if(!packages[p]->versions[t].exists()) continue;
		cout << "  node [" << endl;
		cout << "    id " << p << endl;
		cout << "    label \"" << *packages[p] << "\"" << endl;
		cout << "  ]" << endl;
	}
	
	// Edges
	for(int p=0; p < packages.size(); p++)
	{
		if(!packages[p]->versions[t].exists()) continue;
		vector<Package*>& deps = packages[p]->versions[t].deps;
		for(int d=0; d < deps.size(); d++)
		{
			int depp = package_index(deps[d]);
			cout << "  edge [" << endl;
			cout << "    source " << p << endl;
			cout << "    target " << depp << endl;
			cout << "  ]" << endl;
		}
	}
	cout << "]" << endl;
}
*/

void check_consistency()
{
	// Every package must exists sometime
	for(int p=0; p < dataset.packages().size(); p++)
	{
		Package::Ptr pkg = dataset.package(p);
		if(!pkg->exists())
		{
			// cerr << "Package " << *pkg << " does not exist." << endl;
		}
	}
	
	// Every dependency must exist in the same time
	for(int p=0; p < dataset.packages().size(); p++)
	{
		Package::Ptr pkg = dataset.package(p);
		for(int t=0; t < max_t; t++)
		{
			Version::Ptr version = pkg->version(t);
			
			// If version does not exists than in should not be connected
			if(!version->exists())
			{
				assert(version->in_degree() == 0);
				assert(version->out_degree() == 0);
				continue;
			}
			
			// Should only depend on existing versions
			for(vector<Version::Ptr>::const_iterator d=version->deps().begin();
			d != version->deps().end(); ++d)
			{
				assert((*d)->exists());
			}
			
			// Should only be depended on by existing versions
			for(vector<Version::Ptr>::const_iterator d=version->revdeps().begin();
			d != version->revdeps().end(); ++d)
			{
				assert((*d)->exists());
			}
		}
	}
}

void calculate_growth()
{
	// Timecode, pkgs, deps, newpkgs, newversions, newdeps
	for(int t=1; t < max_t; t++)
	{
		int pkgs = 0;
		int deps = 0;
		int rev = 0;
		int newpkg = 0;
		int delpkg = 0;
		for(int j=0; j < dataset.packages().size(); j++)
		{
			Package::Ptr pkg = dataset.package(j);
			Version::Ptr prevver = pkg->version(t - 1);
			Version::Ptr ver = pkg->version(t);
			
			if(ver->exists()) pkgs++;
			if(!prevver->exists() && ver->exists())
			{
				// cout << "Added " << *ver << endl;
				newpkg++;
			}
			if(prevver->exists() && !ver->exists())
			{
				//cout << "Removed " << clusters[pkg->cluster] << "/" << pkg->label << endl;
				delpkg++;
			}
			if(ver->exists())
			{
				deps += ver->out_degree();;
				rev += ver->in_degree();
			}
		}
		if(pkgs == 0) delpkg = 0;
		cout << t << "\t" << pkgs << "\t" << newpkg << "\t" << delpkg << "\t" << deps << "\t" << rev << endl;
	}
}

vector<Package::Ptr> packages_introduced_in(int t)
{
	vector<Package::Ptr> packages;
	// if(t < 60) return packages;
	for(int j=0; j < dataset.packages().size(); j++)
	{
		Package::Ptr pkg = dataset.package(j);
		if(pkg->cluster()->label() == "virtual-virtual") continue;
		// if(!ends_with(pkg->cluster()->label(), "-libs")) continue;
		Version::Ptr ver = pkg->version(t);
		if(!ver->exists()) continue;
		bool prevver_exists = false;
		for(int p = 0; p < t; ++p)
		{
			Version::Ptr prevver = pkg->version(p);
			if(prevver->exists() || prevver->revdeps().size() > 0) 
			{
				prevver_exists = true;
				break;
			}
			if(pkg->version(p+10)->revdeps().size() > 10) prevver_exists = true;
		}
		if(prevver_exists) continue;
		if(pkg->version(128)->revdeps().size() < 30) continue;
		cout << t << "\t" << *pkg << "\t" << pkg->version(128)->revdeps().size() << endl;
		packages.push_back(pkg);
	}
	return packages;
}

void calculate_fan_in_recurse(Version::Ptr node, set<Version::Ptr>& nodelist, int depth)
{
	pair<set<Version::Ptr>::iterator, bool> ret = nodelist.insert(node);
	if(!ret.second) return; // Node already existed
	if(depth == 0) return;
	depth--;
	for(vector<Version::Ptr>::const_iterator r = node->revdeps().begin(); r != node->revdeps().end(); ++r)
	{
		// if((*r)->package()->cluster()->label() == "virtual-virtual") continue;
		//if(!ends_with((*r)->package()->cluster()->label(), "-libs")) continue;
		// cout << **r << endl;
		calculate_fan_in_recurse(*r, nodelist, depth);
	}
}

set<Version::Ptr> calculate_fan_in(Version::Ptr node, int depth = -1)
{
	set<Version::Ptr> nodelist;
	if(node->exists()) calculate_fan_in_recurse(node, nodelist, depth);
	return nodelist;
}

void calculate_dep_growth(const Package::Ptr& pkg)
{
	cerr << *pkg << endl;
	for(int t=1; t < max_t; t++)
	{
		Version::Ptr ver = pkg->version(t);
		
		cout << t;
		cout << "\t" << calculate_fan_in(ver, 1).size();
		cout << "\t" << calculate_fan_in(ver, 2).size();
		cout << "\t" << calculate_fan_in(ver, 3).size();
		cout << "\t" << calculate_fan_in(ver, 4).size();
		cout << "\t" << calculate_fan_in(ver, 5).size();
		cout << "\t" << calculate_fan_in(ver, 6).size();
		cout << "\t" << calculate_fan_in(ver, 7).size();
		cout << endl;
	}
}

void calculate_average_dep_growth(int depth = 1)
{
	vector<Package::Ptr> pkgs[max_t];
	uint64 num[max_t];
	uint64 sum[max_t]; 
	uint64 sqr[max_t];
	
	for(int s = 40; s < max_t; s++)
	{
		pkgs[s] = packages_introduced_in(s);
	}
	for(int t = 0; t < max_t; t++)
	{
		num[t] = 0;
		sum[t] = 0;
		sqr[t] = 0;
		for(int s = 0 ; s < max_t; s++)
		{
			if(s + t >= max_t) continue;
			num[t] += pkgs[s].size();
			for(vector<Package::Ptr>::iterator i = pkgs[s].begin(); i != pkgs[s].end(); ++i)
			{
				Version::Ptr ver = (*i)->version(s + t);
				uint64 n = calculate_fan_in(ver, depth).size();
				sum[t] += n;
				sqr[t] += n * n;
			}
		}
		double average = sum[t];
		average /= num[t] * num[t];
		double stddev = sqr[t];
		stddev /= num[t] * num[t];
		stddev -= average * average;
		stddev = sqrt(stddev);
		if(num[t] == 0)
		{
			average = stddev = 0.0;
		}
		cout << t << "\t" << average << "\t" << stddev << "\t" << sum[t] << "\t" << num[t] << "\t" << sqr[t] << endl;
	}
}

int main(int argc, char * argv[])
{
	try
	{
		if(argc < 2) throw "Please supply a filename";
		ifstream file(argv[1]);
		
		cerr << "Reading file" << endl;
		const int buffer_size = 100*1024*1024;
		char* buffer = new char[buffer_size];
		file.read(buffer, buffer_size);
		file.close();
		
		char* cursor = buffer;
		dataset.from_bip(cursor);
		
		if(false)
		{
			cerr << "Elliminating metas and virtuals" << endl;
			dataset.elliminate_virtuals();
			dataset.elliminate_metas();
		}
		cerr << "Checking consistency" << endl;
		// check_consistency();
		
		// calculate_growth();
		
		calculate_dep_growth(dataset.package("udev"));
		// calculate_average_dep_growth(1);
		cerr << "Outputting as GML" << endl;
		// dataset.to_gml(cout, 128);
		
		/*
		
		uint64 num_clusters = read_uint(cursor);
		clusters.reserve(num_clusters);
		for(int i=0; i < num_clusters; i++)
		{
			clusters.push_back(read_string(cursor));
		}
		uint64 num_pkg = read_uint(cursor);
		packages.reserve(num_pkg);
		for(int k=0; k < num_pkg; k++)
		{
			packages.push_back(read_package(cursor));
		}
		deserialize_references();
		cerr << "Calculating reverse dependencies" << endl;
		calculate_reverse_deps();
		cerr << "Elliminating virtuals" << endl;
		elliminate_virtuals();
		cerr << "Elliminating metas" << endl;
		elliminate_metas();
		cerr << "Checking consistency" << endl;
		check_consistency();
		
		//dump_graph_gml(48);
		calculate();
		*/
	}
	catch(const char* message)
	{
		cerr << message << endl;
		return -1;
	}
}

