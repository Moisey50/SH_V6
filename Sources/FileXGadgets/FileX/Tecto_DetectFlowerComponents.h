#pragma once
#include "Tecto_ProcessPlantImage.h"

namespace nsTecto
{
    // parameters of processing
    struct DetectFlowerComponentsParameters : protected ProcessPlantImageParameters
    {
        using ProcessPlantImageParameters::MashingSize;
        using ProcessPlantImageParameters::RelationDistance;

        double SmallAreaMaxSize;  // to filter out small conturs.
        double BigAreaMinSize;  // to filter out seed conturs for flower detection.
        double ExtendedContursMaxRatio; // to filter out figures with small area/perimeter
    };

    struct PolylineAttributes
    {
        int conturIndex;
        double area;
        int componentNum;
    };


    class DetectFlowerComponents : public ProcessPlantImage
    {
    protected:
        // read data from stream and process figures.
        void process(std::istream& strm);

        //convert inner result to outside results.
        void PrepareResults();

        // remove conturs with size < parametersDFC.SmallAreaMaxSize
        void RemoveSmallConturs();

        // remove conturs with big area/perimeter ration > parametersDFC.ExtendedContursMaxRatio
        void RemoveExtendedConturs();

        // Compute areas of contours
        void ComputeAreas();

        // Join conturs of flowers.
        void FindConturOfFlowers();

    public:
        DetectFlowerComponents()
        {
            parametersDFC.SmallAreaMaxSize = 200;
            parametersDFC.BigAreaMinSize = 400;
        }

        void SetParameters(const DetectFlowerComponentsParameters& p)
        {
            if (p.MashingSize > 5) parameters.MashingSize = p.MashingSize;
            if (p.RelationDistance > 0) parameters.RelationDistance = p.RelationDistance;
            if (p.SmallAreaMaxSize > 1.0) parametersDFC.SmallAreaMaxSize = p.SmallAreaMaxSize;
            if (p.BigAreaMinSize > 10.0) parametersDFC.BigAreaMinSize = p.BigAreaMinSize;
        }

        // process data from file.
        void process(const char* szMFiguresName);

        // process figures.
        eErrorCode process(const std::vector< std::vector<R2> >&);

    protected:
        int componentCount;
        std::vector<PolylineAttributes> AttributesOfConturs;
        DetectFlowerComponentsParameters parametersDFC;

    };


}