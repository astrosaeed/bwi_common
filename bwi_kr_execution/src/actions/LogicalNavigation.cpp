#include "LogicalNavigation.h"

#include "ActionFactory.h"

#include "actasp/AspFluent.h"

#include <bwi_planning_common/PlannerInterface.h>
#include <bwi_kr_execution/UpdateFluents.h>
#include <bwi_kr_execution/AspFluent.h>

#include <ros/ros.h>

#include <sstream>

using namespace ros;
using namespace std;
using namespace actasp;

namespace bwi_krexec {
	
LogicalNavigation::LogicalNavigation(const std::string& name, const std::vector<std::string>& parameters) :
			name(name),
			parameters(parameters),
			done(false),
      request_in_progress(false) {}

LogicalNavigation::~LogicalNavigation() {
  if (request_in_progress) {
    // The goal was sent but the action is being terminated before being allowed to finish. Cancel that command!
    lnac->cancelGoal();
    delete lnac;
  }
}


struct PlannerAtom2AspFluent {
  bwi_kr_execution::AspFluent operator()(const bwi_planning_common::PlannerAtom& atom) {
    
    bwi_kr_execution::AspFluent fluent;
    fluent.name = atom.name;
    if(!atom.value.empty()) {
      fluent.variables.insert(fluent.variables.end(),atom.value.begin(),atom.value.end());
      
      fluent.timeStep = 0; //the observation does not provide a timeStep
    }
    
    return fluent;
  }
};
    

void LogicalNavigation::run() {
  
  ROS_INFO_STREAM("LogicalNavigation: Executing " << name);

  if (!request_in_progress) {
    lnac = new actionlib::SimpleActionClient<bwi_msgs::LogicalNavigationAction>("execute_logical_goal",
                                                                                                 true);
    lnac->waitForServer();
    goal.command.name = name;
    goal.command.value = parameters;
    lnac->sendGoal(goal);
    request_in_progress = true;
  }

  bool finished_before_timeout = lnac->waitForResult(ros::Duration(0.5f));

	//senseState takes a bit longer than 0.5f to complete, but fluents have to be updated
	if(name == "senseState"){
		ros::Rate wait_rate(0.5f);
		while(!(finished_before_timeout = lnac->waitForResult(ros::Duration(0.5f)))){
			wait_rate.sleep();
		}
	}

  // If the action finished, need to do some work here.
  if (finished_before_timeout) {
		ROS_INFO_STREAM("LogicalNavigation: Updating fluents..");
		
    bwi_msgs::LogicalNavigationResultConstPtr result = lnac->getResult();
    
    // Update fluents based on the result of the logical nav request.
    NodeHandle n;
    ros::ServiceClient krClient = n.serviceClient<bwi_kr_execution::UpdateFluents> ( "update_fluents" );
    krClient.waitForExistence();
    bwi_kr_execution::UpdateFluents uf;
    transform(result->observations.begin(),
              result->observations.end(),
              back_inserter(uf.request.fluents),PlannerAtom2AspFluent());
    krClient.call(uf);

    // Mark the request as completed.
    done = true;

    // Cleanup the simple action client.
    request_in_progress = false;
    delete lnac;
  }
	else {
		ROS_INFO_STREAM("LogicalNavigation: Timeout occurred, not updating fluents");
	}	

}

Action *LogicalNavigation::cloneAndInit(const actasp::AspFluent & fluent) const {
  return new LogicalNavigation(fluent.getName(),fluent.getParameters());
}



// static ActionFactory gothroughFactory(new LogicalNavigation("gothrough"));
// static ActionFactory approachFactory(new LogicalNavigation("approach"));
	
} //namespace
