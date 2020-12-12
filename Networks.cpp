#include "Networks.h"

WindowStruct Window = {};

TimeStruct Time = {};

InputController Input = {};

MouseController Mouse = {};

RandomNumberGenerator Random;

static LogEntry logEntry[MAX_LOG_ENTRIES];
static uint32 logEntryFront = 0;
static uint32 logEntryBack = 0;

static char tmp_string[MAX_LOG_ENTRY_LENGTH];

void log(const char file[], int line, int type, const char* format, ...)
{
	LogEntry &entry = logEntry[logEntryBack % MAX_LOG_ENTRIES];
	entry.type = type;
	entry.time = Time.time;

	// Handle log size limit
	logEntryBack++;
	if (logEntryBack - logEntryFront > MAX_LOG_ENTRIES)
		logEntryFront = logEntryBack - MAX_LOG_ENTRIES;

	// Find base filename (without directories)
	const char *basefile = file;
	const size_t filelen = strlen(file);
	for (size_t i = 0; i < filelen; ++i) {
		if (file[i] == '\\' || file[i] == '/') {
			basefile = file + i + 1;
		}
	}

	// Construct the string from variable arguments
	va_list  ap;
	va_start(ap, format);
	vsprintf_s(tmp_string, MAX_LOG_ENTRY_LENGTH, format, ap);
	va_end(ap);
	sprintf_s(entry.message, MAX_LOG_ENTRY_LENGTH, "%s(%d) : %s\n", basefile, line, tmp_string);

	// Windows debug output
	OutputDebugString(entry.message);
}

uint32 getLogEntryCount()
{
	return logEntryBack - logEntryFront;
}

LogEntry getLogEntry(uint32 logLineIndex)
{
	ASSERT(logEntryFront + logLineIndex < logEntryBack);
	return logEntry[(logEntryFront + logLineIndex) % MAX_LOG_ENTRIES];
}

void clearLogEntries()
{
	logEntryFront = 0;
	logEntryBack = 0;
}

DebugCycleCounter DebugCycleCountersBack[DebugCycleCounter_Count] = {};
DebugCycleCounter DebugCycleCountersFront[DebugCycleCounter_Count] = {};

void DebugSwapCycleCounters()
{
	for (int i = 0; i < DebugCycleCounter_Count; ++i)
	{
		DebugCycleCountersFront[i] = DebugCycleCountersBack[i];
		DebugCycleCountersBack[i].hitCount = 0;
		DebugCycleCountersBack[i].cycleCount = 0;
	}
}
