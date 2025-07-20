//Inputs: n vectors (array), m values each. 
// the index in the array represents frequency
// the values are a structure containing two fields: the brightness and the error (for given confidence level).
// If the error is the same for each measurement, the second field can be omitted and replaced by a constant 


#include "StdAfx.h"
#include "HSCGadget.h"

vector< vector< Value > > getInputs(int nMaterials, int nFrequencies, double error )
{
	// parameters for brightness calculations 
	vector< double > F;
	for( int i = 0; i < nMaterials; ++i )
		F.push_back( i );

	vector< vector< Value > > Inputs;
	for( int m = 0; m < nMaterials; ++m )
	{
		vector< Value > Values;
		for( int f = 0; f < nFrequencies; ++f )
		{
			Value V;
			double a = F[m];
			double b = f * F[m];
			double c = sin( b );
			double d = 10 * error * abs( c );
			V.brightness = d + 0.001; 
			V.error = error;
			Values.push_back( V );
		}
		Inputs.push_back( Values );
	}

	return Inputs;
}



// the fuction returns a vector of indices that correspond the recommended frequencies 
// if threre is no satisfactory solution, returns empty vector (size = 0)
vector< int > getTestingFequencies( vector< vector< Value > > const& Inputs )
{
	int nMaterials = (int) Inputs.size();
	int nFrequencies = (int) Inputs[0].size();
	int nCouples = nMaterials * ( nMaterials - 1 ) / 2;

	Map ComparisonInfos;
	for( int f = 0; f < nFrequencies; ++f )
	{
		for( int m1 = 0; m1 < nMaterials; ++m1 )
		{
			for( int m2 = 0; m2 < m1; ++m2 )
			{
				double Brightness_1 = Inputs[m1][f].brightness;
				double Error_1 = Inputs[m1][f].error;
				double Brightness_2 = Inputs[m2][f].brightness;
				double Error_2 = Inputs[m2][f].error;

				assert( Brightness_1 > 0 );
				assert( Error_1 > 0 );
				assert( Brightness_2 > 0 );
				assert( Error_2 > 0 );

				double clarity = abs( Brightness_1 - Brightness_2 ) - (Error_1 + Error_2);
				if( clarity > 0 )
				{
					ComparisonInfo ci;
					ci.material_1 = m1;
					ci.material_2 = m2;
					ci.clarity = clarity;
					if( ComparisonInfos.count( f ) == 0 )
						ComparisonInfos.insert( pair< int, vector< ComparisonInfo > > ( f, vector< ComparisonInfo >() ) );
					////*************!!!!Check below if found!!!!************
					ComparisonInfos.find( f )->second.push_back( ci );
				}
			}//m2
		}//m1
	}//f

	double MaxClarity = 0;

	vector< Range > Ranges; 
	vector< int >   Counts;
	for( int f = 0; f < nFrequencies; ++f )
	{
		Counts.push_back( (int) ComparisonInfos.count( f ) );
		Range R = ComparisonInfos.equal_range( f );
		Ranges.push_back( R );
	}

	vector< int > frequencies = getBestOverlap( Ranges, Counts, nCouples );
	return frequencies;
}

typedef set< pair< int, int > > Set;

int getNumOfCouplesPresent( vector< Range > const& Ranges )
{
	size_t nRanges = Ranges.size();
	Set Couples;
	for( size_t freq = 0; freq < nRanges; ++freq )
	{
		for( Iterator it = Ranges[freq].first; it != Ranges[freq].second; ++it )
		{
			vector< ComparisonInfo > v = it->second;
			for( size_t i = 0; i < v.size(); ++i )
			{
				pair< int, int > entry;
				entry.first  = v[i].material_1;
				entry.second = v[i].material_2;
				Couples.insert( entry );
			}
		}
		return (int) Couples.size();
	}
	return 0; //???
}

vector< int > getBestOverlap( vector< Range > Ranges, vector< int > Counts, int nCouples )
{
	set< int > result;

	size_t n = Ranges.size();
	vector< Range > R;
	for( size_t iter = 0; iter < n; iter++ )
	{
		int BestCount = -1;
		int iBestRange = -1;
		for( size_t f = 0; f < n; ++f )
		{
			R.push_back( Ranges[ f ] );
			int Count = getNumOfCouplesPresent( R );
			if( BestCount < Count )
			{
				BestCount = Count;
				iBestRange = (int) f;
			}
			R.pop_back();
		}
		if( iBestRange >= 0 )
		{
			R.push_back( Ranges[iBestRange] );
			result.insert( iBestRange );
		}
	}
	int nCouplesPresent = getNumOfCouplesPresent( R );
	if( nCouplesPresent == nCouples )
		return getSetValues( result );
	return vector<int>();
}