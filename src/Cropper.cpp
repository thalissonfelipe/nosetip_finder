//
// Created by marcusv on 10/03/19.
//

#include "Cropper.h"
#include <pcl/io/pcd_io.h>
#include <pcl/kdtree/kdtree_flann.h>

void Cropper::cropByPointIndex(CloudXYZ::ConstPtr &inputCloud,
                               int pointIndex,
                               std::string radiusOrK,
                               float radiusOrKvalue,
                               CloudXYZ::Ptr &outputCloud)
{
    std::vector<int> defaultPointIndexSearch;
    std::vector<float> defaultPointSquaredDistance;

    CloudPC::Ptr defaultPrincipalCurvaturesCloud(new CloudPC());

    defaultPrincipalCurvaturesCloud->points.resize(inputCloud->points.size());

    Cropper::cropByPointIndex(
        inputCloud,
        defaultPrincipalCurvaturesCloud,
        pointIndex,
        radiusOrK,
        radiusOrKvalue,
        outputCloud,
        defaultPrincipalCurvaturesCloud,
        defaultPointIndexSearch,
        defaultPointSquaredDistance);
}

void Cropper::cropByPointIndex(CloudXYZ::ConstPtr &inputCloud,
                               CloudPC::Ptr &inputPrincipalCurvatures,
                               int pointIndex,
                               std::string radiusOrK,
                               float radiusOrKvalue,
                               CloudXYZ::Ptr &outputCloud,
                               CloudPC::Ptr &outputPrincipalCurvaturesCloud,
                               std::vector<int> &pointIndexSearch,
                               std::vector<float> &pointSquaredDistance)
{
    pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;
    kdTree.setInputCloud(inputCloud);

    if (radiusOrK == "radius")
    {
        if (kdTree.radiusSearch(inputCloud->points[pointIndex], radiusOrKvalue, pointIndexSearch, pointSquaredDistance) > 0)
        {
            for (int j = 0; j < pointIndexSearch.size(); j++)
            {
                outputCloud->points.push_back(inputCloud->points[pointIndexSearch[j]]);
                outputPrincipalCurvaturesCloud->points.push_back(inputPrincipalCurvatures->points[pointIndexSearch[j]]);
            }
        }
    }
    else
    {
        if (radiusOrK == "k")
        {
            if (kdTree.nearestKSearch(inputCloud->points[pointIndex], radiusOrKvalue, pointIndexSearch, pointSquaredDistance) > 0)
            {
                for (int j = 0; j < pointIndexSearch.size(); j++)
                {
                    outputCloud->points.push_back(inputCloud->points[pointIndexSearch[j]]);
                    outputPrincipalCurvaturesCloud->points.push_back(inputPrincipalCurvatures->points[pointIndexSearch[j]]);
                }
            }
        }
        else
        {
            throw std::runtime_error("Use only 'radius' or 'k' in cropByPointIndex");
        }
    }
}

void Cropper::cropByPointValues(CloudXYZ::Ptr &inputCloud,
                                float xValue,
                                float yValue,
                                float zValue,
                                std::string radiusOrK,
                                float radiusOrKvalue,
                                CloudXYZ::Ptr &outputCloud)
{
    std::vector<int> defaultPointIndexSearch;
    std::vector<float> defaultPointSquaredDistance;

    CloudPC::Ptr defaultPrincipalCurvaturesCloud(new CloudPC());
    defaultPrincipalCurvaturesCloud->points.resize(inputCloud->points.size());

    defaultPointIndexSearch.resize(inputCloud->points.size());
    defaultPointSquaredDistance.resize(inputCloud->points.size());

    Cropper::cropByPointValues(inputCloud,
                               defaultPrincipalCurvaturesCloud,
                               xValue,
                               yValue,
                               zValue,
                               radiusOrK,
                               radiusOrKvalue,
                               outputCloud,
                               defaultPrincipalCurvaturesCloud,
                               defaultPointIndexSearch,
                               defaultPointSquaredDistance);
}

void Cropper::cropByPointValues(CloudXYZ::Ptr &inputCloud,
                                CloudPC::Ptr &inputPrincipalCurvatures,
                                float xValue,
                                float yValue,
                                float zValue,
                                std::string radiusOrK,
                                float radiusOrKvalue,
                                CloudXYZ::Ptr &outputCloud,
                                CloudPC::Ptr &outputPrincipalCurvaturesCloud,
                                std::vector<int> &pointIndexSearch,
                                std::vector<float> &pointSquaredDistance)
{
    CloudXYZ::ConstPtr constCloud = inputCloud;
    pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;
    kdTree.setInputCloud(constCloud);

    pcl::PointXYZ point = pcl::PointXYZ(xValue, yValue, zValue);

    if (radiusOrK == "radius")
    {
        if (kdTree.radiusSearch(point, radiusOrKvalue, pointIndexSearch, pointSquaredDistance) > 0)
        {
            for (int j = 0; j < pointIndexSearch.size(); j++)
            {
                outputCloud->points.push_back(constCloud->points[pointIndexSearch[j]]);
                outputPrincipalCurvaturesCloud->points.push_back(inputPrincipalCurvatures->points[pointIndexSearch[j]]);
            }
        }

        return;
    }
    else if (radiusOrK == "k")
    {
        if (kdTree.nearestKSearch(point, radiusOrKvalue, pointIndexSearch, pointSquaredDistance) > 0)
        {
            for (int j = 0; j < pointIndexSearch.size(); j++)
            {
                outputCloud->points.push_back(constCloud->points[pointIndexSearch[j]]);
                outputPrincipalCurvaturesCloud->points.push_back(inputPrincipalCurvatures->points[pointIndexSearch[j]]);
            }
        }

        return;
    }
    
    throw std::runtime_error("Use only 'radius' or 'k' in cropByPointIndex");
}

void Cropper::removeIsolatedPoints(CloudXYZ::Ptr &inputCloud,
                                   float radiusSearchSize,
                                   int pointsThreshold,
                                   CloudXYZ::Ptr &outputCloud)
{
    CloudPC::Ptr defaultPrincipalCurvaturesCloud;
    defaultPrincipalCurvaturesCloud->points.resize(inputCloud->points.size());

    Cropper::removeIsolatedPoints(inputCloud,
                                  defaultPrincipalCurvaturesCloud,
                                  radiusSearchSize,
                                  pointsThreshold,
                                  outputCloud,
                                  defaultPrincipalCurvaturesCloud);
}

void Cropper::removeIsolatedPoints(CloudXYZ::Ptr &inputCloud,
                                   CloudPC::Ptr &inputPrincipalCurvatures,
                                   float radiusSearchSize,
                                   int pointsThreshold,
                                   CloudXYZ::Ptr &outputCloud,
                                   CloudPC::Ptr &outputPrincipalCurvaturesCloud)
{
    for (int i = 0; i < inputCloud->points.size(); i++)
    {
        pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;
        kdTree.setInputCloud(inputCloud);

        std::vector<int> pointIndexRadiusSearch;
        std::vector<float> pointRadiusSquaredDistance;

        if (kdTree.radiusSearch(inputCloud->points[i], radiusSearchSize, pointIndexRadiusSearch, pointRadiusSquaredDistance) > 0)
        {
            if (pointIndexRadiusSearch.size() >= pointsThreshold)
            {
                outputCloud->points.push_back(inputCloud->points[i]);
                outputPrincipalCurvaturesCloud->points.push_back(inputPrincipalCurvatures->points[i]);
            }
        }

        pointIndexRadiusSearch.clear();
        pointRadiusSquaredDistance.clear();
    }
}

void Cropper::removeIsolatedPoints(CloudXYZ::Ptr &inputCloud,
                                   std::vector<float> &inputShapeIndex,
                                   float radiusSearchSize,
                                   int pointsThreshold,
                                   bool flexibilizeThresholds,
                                   int minPointsToContinue,
                                   CloudXYZ::Ptr &outputCloud,
                                   std::vector<float> &outputShapeIndex,
                                   CloudsLog &cloudsLog)
{
    int _pointsThreshold = pointsThreshold;

    int iteratorCounter = 1;

    bool continueLoop = true;


    while (continueLoop)
    {
        for (int i = 0; i < inputCloud->points.size(); i++)
        {
            pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;
            kdTree.setInputCloud(inputCloud);

            std::vector<int> pointIndexRadiusSearch;
            std::vector<float> pointRadiusSquaredDistance;

            if (kdTree.radiusSearch(inputCloud->points[i], radiusSearchSize, pointIndexRadiusSearch, pointRadiusSquaredDistance) > 0)
            {
                if (pointIndexRadiusSearch.size() >= _pointsThreshold)
                {
                    outputCloud->points.push_back(inputCloud->points[i]);
                    outputShapeIndex.push_back(inputShapeIndex[i]);
                }
            }

            pointIndexRadiusSearch.clear();
            pointRadiusSquaredDistance.clear();
        }

        std::cout << "Number of points of outputCloud " << outputCloud->points.size() << " Iteration: " << iteratorCounter << std::endl;

        std::string logLabel = "3." + std::to_string(iteratorCounter) + " Removing isolated points (min " + std::to_string(_pointsThreshold) + ")";
        cloudsLog.add(logLabel, outputCloud);

        if (outputCloud->points.size() < minPointsToContinue && flexibilizeThresholds)
        {
            _pointsThreshold--;

            outputCloud->points.clear();
            outputShapeIndex.clear();
        }
        else
        {
            continueLoop = false;
        }

        iteratorCounter++;
    }
}
