/***********************************************************************
 **  radGraph.h
 ***********************************************************************/

#ifndef RADGRAPH_H
#define RADGRAPH_H

#include "radimgfunc.h"
#include "itkPoint.h"

#include <set> 
#include <vector>


namespace rad_graph
{
	class Vertex;
	typedef std::vector< Vertex > Vertices;
	typedef std::set <int> Neighbours;
	
    class Vertex 
	{
	private:
		int vertex_id;
	public:
		Neighbours neighbours;
		vector<double> weights;
		ShortPointType2D pt;

		bool noise;
		bool visited;
		int clustering_label;

		Vertex( int d )
		{
            vertex_id = d;
			pt[0] = pt[1] = 0;
			noise = false;
			visited = false;
			clustering_label = 0;
		}
		
		Vertex( )
		{
            vertex_id = 0;
			pt[0] = pt[1] = 0;
			noise = false;
			visited = false;
			clustering_label = 0;
		}
	};

	class Graph
	{
	public:
		void addEdgeIndices ( int index1, int index2 ) 
		{
			vertices[ index1 ].neighbours.insert( index2 );
			double weight = sqrt((double)(vertices[ index1 ].pt[0]-vertices[ index2 ].pt[0]) *(vertices[ index1 ].pt[0]-vertices[ index2 ].pt[0]) 
				+ (double)(vertices[ index1 ].pt[1]-vertices[ index2 ].pt[1]) *(vertices[ index1 ].pt[1]-vertices[ index2 ].pt[1]));
			vertices[ index1 ].weights.push_back(weight);

			//add by my own
			vertices[ index2 ].neighbours.insert( index1 );
			vertices[ index2 ].weights.push_back(weight);
		}

		bool findVertexIndex( int n1, int n2 )
		{
			return (vertices[n1].neighbours.find(n2) == vertices[n1].neighbours.end());
			
			
			/*std::vector<Vertex>::iterator it;
			Vertex v(val);
			it = std::find( vertices.begin(), vertices.end(), v );
			if (it != vertices.end())
			{
				res = true;
				return it;
			} 
			else 
			{
				res = false;
				return vertices.end();
			}*/
		}

		void addEdge ( int n1, int n2 ) 
		{

			/*bool foundNet1 = false, foundNet2 = false;
			Vertices::iterator vit1 = findVertexIndex( n1, foundNet1 );
			int node1Index = -1, node2Index = -1;

			if ( !foundNet1 ) 
			{
				Vertex v1( n1 );
				v1.pt[0] = pt1[0];
				v1.pt[1] = pt1[1];
				vertices.push_back( v1 );
				node1Index = vertices.size() - 1;
			} 
			else 
			{
				node1Index = vit1 - vertices.begin();
			}
			
			Vertices::iterator vit2 = findVertexIndex( n2, foundNet2);
			if ( !foundNet2 ) 
			{
				Vertex v2( n2 );
				v2.pt[0] = pt2[0];
				v2.pt[1] = pt2[1];

				vertices.push_back( v2 );
				node2Index = vertices.size() - 1;
			} 
			else 
			{
				node2Index = vit2 - vertices.begin();
			}

			assert( ( node1Index > -1 ) && ( node1Index <  vertices.size()));
			assert( ( node2Index > -1 ) && ( node2Index <  vertices.size()));*/

			if (findVertexIndex( n1, n2 ))
				addEdgeIndices( n1, n2 );
		}

		Vertices & GetVertices() {return vertices;}
	private :
		Vertices vertices;
	};

} // End of namespace rad_graph

#endif // RADGRAPH_H
