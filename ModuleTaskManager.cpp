#include "Networks.h"


inline void enqueue(Task **queue, int *front, int *back, int max_elems, Task * task)
{
	ASSERT(*back != *front);
	queue[*back] = task;
	if (*front == -1) {
		*front = *back;
	}
	*back = (*back + 1) % max_elems;
}

inline Task *dequeue(Task **queue, int *front, int *back, int max_elems)
{
	ASSERT(*front != -1);
	Task *task = queue[*front];
	queue[*front] = nullptr;
	*front = (*front + 1) % max_elems;
	if (*front == *back) {
		*front = -1;
		*back = 0;
	}
	return task;
}


static const int MAX_THREADS = 4;
static std::thread threads[MAX_THREADS];

static std::mutex scheduledMutex;
static std::mutex finishedMutex;
static std::condition_variable event;

void ModuleTaskManager::threadMain()
{
	Task *task = nullptr;
	
	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(scheduledMutex);
			while (scheduledTasksFront == -1 && exitFlag == false)
			{
				event.wait(lock);
			}

			if (exitFlag)
			{
				break;
			}
			else
			{
				task = dequeue(scheduledTasks,
							   &scheduledTasksFront,
							   &scheduledTasksBack,
							   MAX_TASKS);
			}
		}

		task->execute();

		{
			std::unique_lock<std::mutex> lock(finishedMutex);

			enqueue(finishedTasks,
					&finishedTasksFront,
					&finishedTasksBack,
					MAX_TASKS,
					task);
		}
	}
}

bool ModuleTaskManager::init()
{
	for (auto &thread : threads)
	{
		thread = std::thread(&ModuleTaskManager::threadMain, this);
	}

	return true;
}

bool ModuleTaskManager::update()
{
	if (finishedTasksFront != -1)
	{
		std::unique_lock<std::mutex> lock(finishedMutex);

		while (finishedTasksFront != -1)
		{
			Task *task = dequeue(finishedTasks,
								 &finishedTasksFront,
								 &finishedTasksBack,
								 MAX_TASKS);

			task->owner->onTaskFinished(task);
		}
	}

	return true;
}

bool ModuleTaskManager::cleanUp()
{
	{
		std::unique_lock<std::mutex> lock(scheduledMutex);
		exitFlag = true;
		event.notify_all();
	}

	for (auto &thread : threads)
	{
		thread.join();
	}

	return true;
}

void ModuleTaskManager::scheduleTask(Task *task, Module *owner)
{
	task->owner = owner;

	std::unique_lock<std::mutex> lock(scheduledMutex);

	enqueue(scheduledTasks,
			&scheduledTasksFront,
			&scheduledTasksBack,
			MAX_TASKS,
			task);

	event.notify_one();
}
