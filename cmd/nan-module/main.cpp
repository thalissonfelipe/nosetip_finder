
//
// Created by marcusv on 17/02/21.
//
#include <iostream>
#include <vector>
#include <cmath>
#include <string.h>
#include <fstream>

#include <pcl/io/pcd_io.h>
#include <pcl/conversions.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/common/transforms.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/principal_curvatures.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/filters/filter.h>

#include <nan.h>

#include "../../src/Main.h"
#include "../../src/CloudsLog.h"

v8::Local<v8::Array> parsePointCloudToV8Array(pcl::PointCloud<pcl::PointXYZ> pointCloud)
{
    int pointCloudSize = pointCloud.points.size();
    v8::Local<v8::Array> response = Nan::New<v8::Array>(pointCloudSize);

    for (int i = 0; i < pointCloudSize; i++)
    {
        pcl::PointXYZ point;
        v8::Local<v8::Object> obj = Nan::New<v8::Object>();

        point = pointCloud.points[i];

        obj->Set(Nan::New("x").ToLocalChecked(), Nan::New<v8::Number>(point.x));
        obj->Set(Nan::New("y").ToLocalChecked(), Nan::New<v8::Number>(point.y));
        obj->Set(Nan::New("z").ToLocalChecked(), Nan::New<v8::Number>(point.z));
        Nan::Set(response, i, obj);
    }

    return response;
}

v8::Local<v8::Array> cloudsLogsEntriestoV8Array(std::vector<CloudsLogEntry> logs)
{
    int logsSize = logs.size();
    v8::Local<v8::Array> response = Nan::New<v8::Array>(logsSize);

    for (int i = 0; i < logsSize; i++)
    {
        CloudsLogEntry log = logs[i];

        v8::Local<v8::Object> obj = Nan::New<v8::Object>();
        obj->Set(Nan::New("cloud_label").ToLocalChecked(), Nan::New(log.cloudLabel).ToLocalChecked());
        obj->Set(Nan::New("cloud").ToLocalChecked(), parsePointCloudToV8Array(log.cloud));

        Nan::Set(response, i, obj);
    }

    return response;
}

NAN_METHOD(FindFiducialPoint)
{
    try
    {
        std::string filename(*Nan::Utf8String(info[0]));
        bool flexibilizeThresholds = info[1]->BooleanValue();
        bool flexibilizeCrop = info[2]->BooleanValue();
        int computationRadiusOrKSize = info[3]->NumberValue();
        std::string computationMethod(*Nan::Utf8String(info[4]));
        double minGaussianCurvature = info[5]->NumberValue();
        double shapeIndexLimit = info[6]->NumberValue();
        float minCropSize = info[7]->NumberValue();
        float maxCropSize = info[8]->NumberValue();
        int minPointsToContinue = info[9]->NumberValue();
        float removeIsolatedPointsRadius = info[10]->NumberValue();
        int removeIsolatedPointsThreshold = info[11]->NumberValue();
        int nosetipSearchRadius = info[12]->NumberValue();
        float gfSearchRadius = info[13]->NumberValue();
        std::string features(*Nan::Utf8String(info[14]));
        std::string featuresThreshold(*Nan::Utf8String(info[15]));
        std::string fiducialPoint(*Nan::Utf8String(info[16]));

        struct MainResponse response = Main::run(
            filename,
            flexibilizeThresholds,
            flexibilizeCrop,
            computationRadiusOrKSize,
            computationMethod,
            minGaussianCurvature,
            shapeIndexLimit,
            minCropSize,
            maxCropSize,
            minPointsToContinue,
            removeIsolatedPointsRadius,
            removeIsolatedPointsThreshold,
            nosetipSearchRadius,
            gfSearchRadius,
            features,
            featuresThreshold,
            fiducialPoint);

        pcl::PointXYZ point = response.point;
        CloudsLog cloudsLog = response.cloudsLog;

        v8::Local<v8::Object> pointV8Object = Nan::New<v8::Object>();
        pointV8Object->Set(Nan::New("x").ToLocalChecked(), Nan::New<v8::Number>(point.x));
        pointV8Object->Set(Nan::New("y").ToLocalChecked(), Nan::New<v8::Number>(point.y));
        pointV8Object->Set(Nan::New("z").ToLocalChecked(), Nan::New<v8::Number>(point.z));

        v8::Local<v8::Object> moduleResponse = Nan::New<v8::Object>();
        moduleResponse->Set(Nan::New("point").ToLocalChecked(), pointV8Object);
        moduleResponse->Set(Nan::New("intermediary_clouds").ToLocalChecked(), cloudsLogsEntriestoV8Array(cloudsLog.getLogs()));

        info.GetReturnValue().Set(moduleResponse);
    }
    catch (std::exception &e)
    {
        v8::Isolate* isolate = v8::Isolate::GetCurrent();
        isolate->ThrowException(v8::Exception::TypeError(v8::String::NewFromUtf8(isolate, e.what())));
    }
}

using Nan::GetFunction;
using Nan::New;
using Nan::Set;
using v8::Boolean;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Number;
using v8::Object;
using v8::String;

NAN_MODULE_INIT(init)
{
    Set(target, New<String>("findFiducialPoint").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(FindFiducialPoint)).ToLocalChecked());
}

NODE_MODULE(fiducial_point_finder, init)