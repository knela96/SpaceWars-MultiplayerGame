#pragma once

class Task
{
public:

	virtual void execute() = 0;

	Module *owner = nullptr;
};

class ModuleTaskManager : public Module
{
public:

	// Virtual method

	bool init() override;

	bool update() override;

	bool cleanUp() override;


	// To schedule new tasks

	void scheduleTask(Task *task, Module *owner);

	void threadMain();

private:

	Task* scheduledTasks[MAX_TASKS];
	int scheduledTasksFront = -1;
	int scheduledTasksBack = 0;

	Task* finishedTasks[MAX_TASKS];
	int finishedTasksFront = -1;
	int finishedTasksBack = 0;

	bool exitFlag = false;
};
