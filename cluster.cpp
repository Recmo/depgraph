#include "cluster.h"
#include "varint.h"

Cluster::Cluster()
{
}

Cluster::~Cluster()
{
}

void Cluster::to_bip(char*& cursor)
{
	
}

void Cluster::from_bip(char*& cursor)
{
	set_label(read_string(cursor));
}

ostream& operator<<(ostream& out, const Cluster& in)
{
	out << in.label();
	return out;
}

