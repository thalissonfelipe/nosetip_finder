//
// Created by marcusv on 10/03/19.
//

#include <pcl/console/print.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/io/pcd_io.h>
#include "NosetipFinder.h"

void NosetipFinder::thresholdByShapeIndex(CloudXYZ::Ptr &inputCloud, std::vector<float> shapeIndexes,
                                          float thresholdMin, float thresholdMax,
                                          CloudXYZ::Ptr &outputCloud,
                                          std::vector<float> outputShapeIndexes)
{
    if (inputCloud->points.size() != shapeIndexes.size())
    {
        throw std::runtime_error("Input Cloud and Shape Indexes Vector must have the same size in thresholdByShapeIndex");
        return;
    }

    for (int i = 0; i < shapeIndexes.size(); i++)
    {
        if ((shapeIndexes[i] > thresholdMin) && (shapeIndexes[i] < thresholdMax))
        {
            outputCloud->push_back(inputCloud->points[i]);
            outputShapeIndexes.push_back(shapeIndexes[i]);
        }
    }
}

void NosetipFinder::thresholdByGaussianCurvature(CloudXYZ::Ptr &inputCloud,
                                                 CloudPC::Ptr
                                                     &inputPrincipalCurvaturesCloud,
                                                 float thresholdMin,
                                                 CloudXYZ::Ptr &outputCloud,
                                                 CloudPC::Ptr
                                                     &outputPrincipalCurvaturesCloud)
{
    if (inputCloud->points.size() != inputPrincipalCurvaturesCloud->points.size())
    {
        throw std::runtime_error("Input Cloud and Principal Curvatures Vector must have the same size in thresholdByGaussianCurvature");
        return;
    }

    float k1, k2;

    for (int i = 0; i < inputPrincipalCurvaturesCloud->size(); i++)
    {
        k1 = inputPrincipalCurvaturesCloud->points[i].pc1;
        k2 = inputPrincipalCurvaturesCloud->points[i].pc2;

        if (k1 * k2 > thresholdMin)
        {
            outputCloud->push_back(inputCloud->points[i]);
            outputPrincipalCurvaturesCloud->push_back(inputPrincipalCurvaturesCloud->points[i]);
        }
    }
}

void NosetipFinder::thresholdByShapeIndexAndGaussianCurvature(CloudXYZ::Ptr &inputCloud,
                                                              std::vector<float> shapeIndexes,
                                                              CloudPC::Ptr
                                                                  &inputPrincipalCurvaturesCloud,
                                                              float thresholdShapeIndexMin,
                                                              float thresholdShapeIndexMax,
                                                              float thresholdPrincipalCurvatureMin,
                                                              CloudXYZ::Ptr &outputCloud,
                                                              std::vector<float> outputShapeIndexes,
                                                              CloudPC::Ptr
                                                                  &outputPrincipalCurvaturesCloud)
{
    if (
        (inputCloud->points.size() != shapeIndexes.size()) || (inputCloud->points.size() != inputPrincipalCurvaturesCloud->points.size()))
    {
        throw std::runtime_error("Input Cloud, Shape Indexes Vector and Principal Curvatures Cloud must have the same size.");
        return;
    }

    float k1, k2;

    for (int i = 0; i < shapeIndexes.size(); i++)
    {
        if ((shapeIndexes[i] > thresholdShapeIndexMin) && (shapeIndexes[i] < thresholdShapeIndexMax))
        {
            k1 = inputPrincipalCurvaturesCloud->points[i].pc1;
            k2 = inputPrincipalCurvaturesCloud->points[i].pc2;

            if (k1 * k2 > thresholdPrincipalCurvatureMin)
            {
                outputCloud->push_back(inputCloud->points[i]);
                outputPrincipalCurvaturesCloud->push_back(inputPrincipalCurvaturesCloud->points[i]);
                outputShapeIndexes.push_back(shapeIndexes[i]);
            }
        }
    }
}

pcl::PointXYZ NosetipFinder::chooseANoseTip(CloudXYZ::Ptr inputCloud,
                                            std::vector<float> shapeIndexes,
                                            int thresholdPoints,
                                            CloudsLog &cloudsLog)
{
    if (thresholdPoints > inputCloud->points.size())
    {
        throw std::runtime_error("Your threshold of points is to large for this input cloud");
        return pcl::PointXYZ();
    }

    if (inputCloud->points.size() != shapeIndexes.size())
    {
        throw std::runtime_error("Your input cloud and input shape indexes must have the same size");
        return pcl::PointXYZ();
    }

    pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;

    CloudXYZ::Ptr clonedCloud(new CloudXYZ);
    CloudPC::Ptr clonedPC(new CloudPC);
    std::vector<float> clonedSI;

    for (int i = 0; i < inputCloud->points.size(); i++)
    {
        clonedCloud->points.push_back(inputCloud->points[i]);
        clonedSI.push_back(shapeIndexes[i]);
    }

    std::vector<int> lowestSIIndices(thresholdPoints);
    int toDelete;

    pcl::PointXYZ noseTip;
    pcl::PrincipalCurvatures principalCurvatureOfNoseTip;
    float nosetipSI;

    for (int j = 0; j < thresholdPoints; j++)
    {
        for (int i = 0; i < clonedCloud->points.size(); i++)
        {
            float iterationSI = shapeIndexes[i];

            if (i == 0)
            {
                noseTip = clonedCloud->points[0];
                principalCurvatureOfNoseTip = clonedPC->points[0];
                nosetipSI = clonedSI[0];
                lowestSIIndices[j] = 0;
            }
            else
            {
                if (iterationSI < nosetipSI)
                {
                    noseTip = clonedCloud->points[i];
                    nosetipSI = clonedSI[i];
                    lowestSIIndices[j] = i;
                }
            }
        }

        toDelete = lowestSIIndices[j];

        for (int i = 0; i < inputCloud->points.size(); i++)
        {
            if (inputCloud->points[i].x == clonedCloud->points[lowestSIIndices[j]].x && inputCloud->points[i].y == clonedCloud->points[lowestSIIndices[j]].y && inputCloud->points[i].z == clonedCloud->points[lowestSIIndices[j]].z)
            {
                lowestSIIndices[j] = i;
                break;
            }
        }

        clonedCloud->points.erase(clonedCloud->points.begin() + toDelete);
        clonedSI.erase(clonedSI.begin() + toDelete);
    }

    std::vector<std::vector<int>> points_index_vector(thresholdPoints);
    std::vector<std::vector<float>> points_rsd_vector(thresholdPoints);

    kdTree.setInputCloud(inputCloud);

    CloudXYZ::Ptr pointsWithLowerShapeIndex(new CloudXYZ);

    for (int i = 0; i < lowestSIIndices.size(); i++)
    {
        pointsWithLowerShapeIndex->points.push_back(inputCloud->points[lowestSIIndices[i]]);
        if (kdTree.radiusSearch(inputCloud->points[lowestSIIndices[i]], 20, points_index_vector[i], points_rsd_vector[i]) > 0)
        {
            std::cout << "Searching in the neighboorhod of the point " << i << " of largest curvature." << std::endl;
        }
        else
        {
            throw std::runtime_error("Couldn't search points");
            return pcl::PointXYZ();
        }
    }

    cloudsLog.add("4. " + std::to_string(lowestSIIndices.size()) + " lowest shape index points", pointsWithLowerShapeIndex);

    int biggest_index = -1;

    for (int i = 0; i < lowestSIIndices.size(); i++)
    {
        if (i == 0)
        {
            biggest_index = 0;
        }
        else
        {
            if (points_index_vector[i].size() > points_index_vector[biggest_index].size())
            {
                biggest_index = i;
            }
        }
    }

    noseTip = inputCloud->points[lowestSIIndices[biggest_index]];

    return noseTip;
}

bool NosetipFinder::itsAGoodNoseTip(pcl::PointXYZ noseTip, float xValue, float yValue, float zValue, float maxDistance)
{
    double distance = sqrt(pow((noseTip.x - xValue), 2) + pow((noseTip.y - yValue), 2) + pow((noseTip.z - zValue), 2));
    return distance <= maxDistance;
}

void NosetipFinder::removeNonExistingIndices(CloudXYZ::Ptr &inputCloud, std::vector<int> indicesToKeep)
{
    CloudXYZ::Ptr tempCloud(new CloudXYZ);

    for (int i = 0; i < inputCloud->points.size(); i++)
    {
        tempCloud->points.push_back(inputCloud->points[i]);
    }

    inputCloud->points.clear();

    for (int i = 0; i < indicesToKeep.size(); i++)
    {
        inputCloud->points.push_back(tempCloud->points[indicesToKeep[i]]);
    }
}

void NosetipFinder::removeNonExistingIndices(CloudPC::Ptr &inputCloud, std::vector<int> indicesToKeep)
{
    CloudPC::Ptr tempCloud(new CloudPC);

    for (int i = 0; i < inputCloud->points.size(); i++)
    {
        tempCloud->points.push_back(inputCloud->points[i]);
    }

    inputCloud->points.clear();

    for (int i = 0; i < indicesToKeep.size(); i++)
    {
        inputCloud->points.push_back(tempCloud->points[indicesToKeep[i]]);
    }
}

bool NosetipFinder::saveNoseTip(pcl::PointXYZ noseTip, std::string filename, std::string cloudName)
{
    if (filename.substr(filename.length() - 3) == "pcd")
    {
        CloudXYZ::Ptr noseTipCloud(new CloudXYZ);

        float bad_point = std::numeric_limits<float>::quiet_NaN();
        pcl::PointXYZ nan_point = pcl::PointXYZ(bad_point, bad_point, bad_point);

        for (int i = 0; i <= 13; i++)
        {
            if (i == 13)
            {
                noseTipCloud->push_back(noseTip);
            }
            else
            {
                noseTipCloud->push_back(nan_point);
            }
        }

        pcl::io::savePCDFile(filename, *noseTipCloud);
        return true;
    }
    else
    {
        if (filename.substr(filename.length() - 3) == "txt")
        {
            std::ofstream ofs;
            ofs.open(filename.c_str(), std::ios_base::app);
            if (ofs.is_open())
            {
                ofs << cloudName << " " << noseTip << std::endl;
                ofs.close();
                return true;
            }
            else
            {
                return false;
            }
        }
    }
}