#pragma once

#include <vector>
#include <iostream>

#include "Tecto_Polyline.h"

typedef void( *TectoLogCB )( const char * pLogString ) ;


namespace nsTecto
{
    class VDrawingInterface
    {
    public:
        virtual void showPolyline(int idx, const std::vector<R2>& pl) = 0;
        virtual void DrawPolyline(int idx, const std::vector<R2>& pl) = 0;
        virtual void Show(int delayMs = 0) = 0;
    };

    // parameters of processing
    struct ProcessPlantImageParameters
    {
        int MashingSize; // to speed up computing distance between two 
                         // fugures. The figures is splitted to subfigures
                         // with count of sides <= MashingSize lines. 

        double XAxeCompressCoeff; // to scale figures. Algorithm scale the fugures before any searching.
                                  // This allows to perform searching in  x*x + y*y metric.
        double RelationDistance;  // if distance between figures is < sqrt( RelationDistance ), they are related
                                  // Attention. The value is applied to the scaled fugures and this value is squared.

        double SlicedPartForStalk; // the percent of the outer rectange of the image sliced to detect 
                                   // stalk
        int StalkSlicesCutOffcoeff; // minimum stalk slices necessary to estimate stalk. 

        int MinStalkSlicedCount; // count of slice to estimate stalk
    };

    // relation informaton with indices of figures.
    struct Relation : public PolylinesDistance
    {
        int idx0, idx1;
    };

    // slice information: x abscissa of vertical axe, y is middle of slice, ylength - length of slice.
    struct StalkSlice : public R2
    {
        int count; // count of intersectd conturs.
        double ylength;
    };

    // below class ProcessPlantImage detect parts of a plant among
    // given conturs and estunate most fitted rectanble for the stalk of the plant.
    // results of processing is stored to ProcessPlantImageResults.

    // results of processing
    struct ProcessPlantImageResults
    {
        Rectangle boundary; // outer boundary of detected plant.

        int ReferenceConturIndex; // reference contur index ( contur with maximum area ).
        std::vector<int> includedConturs; // indices of conturs forming plant image.
        std::vector<int> LengthlyConturs; // indices of long conturs.
        std::vector<Relation> relations;  // information about relation conturs forming plang image 

        double stalkWidth;  // stalk width
        std::vector<R2> stalkContur; // rectangle - stalk contur.
    };

    class ProcessPlantImage
    {
    public:
        typedef enum enumErrorCode
        {
            OK, FAIL
        }eErrorCode;

    protected:
        // read data from stream and process figures.
        void process(std::istream& strm);

        // read data from stream
        static std::vector< std::vector<R2> > readFigures(std::istream&);
        // convert figures from std::vector<R2> to Polylines
        static void copyToPolylines(const std::vector< std::vector<R2> >& pl, std::vector< Polyline >& f);

        // sometime the input figures have duplicated vertices. the function remove duplicates
        void RemoveDuplicateVertices();
        // rotate vertices till first is first in lexicographic order. Necessary to remove duplcated
        // conturs.
        void SetStartingPoints();
        // sometime the input figures have duplicates. The function leave only unique figures
        void RemoveDuplicatedConturs();

        // scale fugures by given value from process parameters.
        void ScaleFigures();

        // duplicate the start of polyline to the end. This allows to use
        // p[size()-2], p[size()-1] instead of p[size()-1 ,p[0]. The first one 
        // don't need index rollover checking.
        void ConvertPolygonsToPolylines();

        // find outer rectangles for conturs.
        void FindOuterRectangles();

        // find reference contur ( with maximum area ).
        void FindBasePolygon();

        // mash the conturs. The contur is splitted to smaller polylines with a count 
        // of segments from process parameters.
        void MashPolygons();

        // Join conturs to plant.
        void GetConnectedComponents();
        // Get outer rectangle of the plant contur.
        void GetConnectedComponentsBoundary();

        //slice plant contur from start of x axe.
        void GetStalkSlices();
        //get stalk rectangle from slices.
        void EstimateStalkRectangle();

        //convert inner result to outside results.
        void PrepareResults();

        void addRelation(int idx0, int id1, const PolylinesDistance& relation);

        // helper function for drawing.
        std::vector<R2> RescaleFigure(const Polyline& pl) const;

    public:
        ProcessPlantImage()
            : IdxMaxArea(-1)
            , drawer(NULL)
        {
            parameters.MashingSize = 10;
            parameters.XAxeCompressCoeff = 1.0;
            parameters.RelationDistance = 2.0;
            parameters.SlicedPartForStalk = 0.1;
            parameters.StalkSlicesCutOffcoeff = 2;
            parameters.MinStalkSlicedCount = 20;
        }

        // process data from file.
        void process(const char* szMFiguresName );
        // process figures.
        eErrorCode process( const std::vector< std::vector<R2> >& );


        void SetDrawingInterface(VDrawingInterface& d)
        {
            drawer = &d;
        }

        void SetParameters(const ProcessPlantImageParameters& p)
        {
            if (p.MashingSize > 5) parameters.MashingSize = p.MashingSize;
            if (p.XAxeCompressCoeff > 0) parameters.XAxeCompressCoeff = p.XAxeCompressCoeff;
            if (p.RelationDistance > 0) parameters.RelationDistance = p.RelationDistance;
            if (p.SlicedPartForStalk > 0.05 ) parameters.SlicedPartForStalk = p.SlicedPartForStalk;
            if (p.StalkSlicesCutOffcoeff >= 2 ) parameters.StalkSlicesCutOffcoeff = p.StalkSlicesCutOffcoeff;
            if (p.MinStalkSlicedCount >= 10) parameters.MinStalkSlicedCount = p.MinStalkSlicedCount;
        }

        const ProcessPlantImageResults& Results() const { return results; }

    protected:
        std::vector <int> InitialIndices; // during removing duplicate figures indices shifter. 
        std::vector< Polyline > figures; // list of figures 

        int IdxMaxArea;  // index of reference figure.
        std::vector<int> includedConturs; // index of components forming the plant from connected figures.
        Rectangle connectedComponentsBoundary; // outer boundary of the plant from connected figures.

        std::vector< Relation > relations; // how figures connected to form the plant.

        std::vector<StalkSlice> stalkSlices; // temporary data to keep slices to estimate a stalk.
        std::vector<R2> stalk;               // the estimated stalk.
        double stalkWidth;                   // the estimated stalk width 

        VDrawingInterface* drawer;   // drawing interface. 

        ProcessPlantImageParameters parameters; // input parameters.
        ProcessPlantImageResults results;       // output results.
    };

}
