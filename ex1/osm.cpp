#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "osm.h"

int osm_init(){
    return 0;
}

void foo(){
    return;
}

double osm_syscall_time(unsigned int osm_iterations){

    if (osm_iterations == 0){
        return -1;
    }

    int remainder = osm_iterations%100;
    unsigned int limit = remainder==0 ? osm_iterations: osm_iterations/100 +1;

    struct timeval tim;
    gettimeofday(&tim, NULL);
    double start_nano=((tim.tv_sec*1000000)+tim.tv_usec)*1000;

    for (int i=1;i<limit;++i){
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
        OSM_NULLSYSCALL;
    }

    gettimeofday(&tim, NULL);
    double end_nano=((tim.tv_sec*1000000)+tim.tv_usec)*1000;

    return (end_nano-start_nano)/(limit*100);
}

double osm_function_time(unsigned int osm_iterations){

    if (osm_iterations == 0){
        return -1;
    }

    int remainder = osm_iterations%100;
    unsigned int limit = remainder==0 ? osm_iterations: osm_iterations/100 +1;

    struct timeval tim;
    gettimeofday(&tim, NULL);
    double start_nano=((tim.tv_sec*1000000)+tim.tv_usec)*1000;

    for (int i=1;i<limit;++i){
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
        foo();
    }

    gettimeofday(&tim, NULL);
    double end_nano=((tim.tv_sec*1000000)+tim.tv_usec)*1000;

    return (end_nano-start_nano)/(limit*100);
}

double osm_operation_time(unsigned int osm_iterations){
    if (osm_iterations == 0){
        return -1;
    }

    int remainder = osm_iterations%100;
    unsigned int limit = remainder==0 ? osm_iterations: osm_iterations/100 +1;
    int a;


    struct timeval tim;
    gettimeofday(&tim, NULL);
    double start_nano=((tim.tv_sec*1000000)+tim.tv_usec)*1000;

    for (int i=1;i<limit;++i){
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
        a=4+7;
    }

    gettimeofday(&tim, NULL);
    double end_nano=((tim.tv_sec*1000000)+tim.tv_usec)*1000;

    return (end_nano-start_nano)/(limit*100);
}

timeMeasurmentStructure measureTimes (unsigned int osm_iterations){
    if (osm_iterations == 0){
        osm_iterations = 50000;
    }

    int remainder = osm_iterations%100;
    unsigned int limit = remainder==0 ? osm_iterations: osm_iterations/100 +1;
    unsigned int actualIterations = limit*100;

    timeMeasurmentStructure measurmentStructure= timeMeasurmentStructure();

    measurmentStructure.functionTimeNanoSecond= osm_function_time(osm_iterations);
    measurmentStructure.instructionTimeNanoSecond = osm_operation_time(osm_iterations);
    measurmentStructure.trapTimeNanoSecond= osm_syscall_time(osm_iterations);

    measurmentStructure.numberOfIterations= actualIterations;

    if (measurmentStructure.instructionTimeNanoSecond==-1 || measurmentStructure.functionTimeNanoSecond==-1){
        measurmentStructure.functionInstructionRatio=-1;
    }
    else{
        measurmentStructure.functionInstructionRatio=measurmentStructure.functionTimeNanoSecond/
                                measurmentStructure.instructionTimeNanoSecond;
    }

    if (measurmentStructure.instructionTimeNanoSecond==-1 || measurmentStructure.trapTimeNanoSecond==-1){
        measurmentStructure.trapInstructionRatio=-1;
    }
    else{
        measurmentStructure.trapInstructionRatio=measurmentStructure.trapTimeNanoSecond/
                measurmentStructure.instructionTimeNanoSecond;
    }

    int hostnameSuccess = gethostname(measurmentStructure.machineName, HOST_NAME_MAX-1);
    if (hostnameSuccess!=0){
        strcpy(measurmentStructure.machineName,"");
    }

    return measurmentStructure;

}