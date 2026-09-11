#pragma once

namespace DevBenchTool
{
	// Registers the read-only "infinityui.state" tool with DevBench when it is present. Call with false at
	// kPostLoad and true at kDataLoaded (DevBench may register its interface after this plugin loads).
	void Init(bool a_lastAttempt);
}
