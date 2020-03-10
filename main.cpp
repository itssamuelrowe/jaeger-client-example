#include <iostream>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>

using namespace std;
using namespace opentracing;
using namespace jaegertracing::logging;
using namespace YAML;

void setUpTracer(const char* path) {
	auto file = LoadFile(path);
	auto config = Config::parse(file);
	auto logger = consoleLogger();
	auto tracer = Tracer::make("service", config, logger);
	Tracer::InitGlobal(static_pointer_cast<Tracer>(tracer));

	cout << "[info] The tracer was successfully initialized.\n";
}

void shutdownTracer() {
	auto globalTracer = Tracer::Global();
	globalTracer->Close();

	cout << "[info] The tracer was successfully shutdown.\n";
}

unique_ptr<Span> addSpan(const char* name) {
	auto globalTracer = Tracer::Global();
	auto span = globalTracer->StartSpan(name);

	return span;
}

unique_ptr<Span> addChildSpan(const unique_ptr<Span>& parent, const char* name) {
	auto globalTracer = Tracer::Global();
	auto parentContext = parent->Context();
	auto child = ChildOf(parentContext);
	auto span = globalTracer->StartSpan(name, { child });

	cout << "[info] Successfully created tracer named '" << name << "'.\n";

	return span;
}

void function1() {
	auto span = addSpan("function1");

	function2(span);
	function3(span);

	cout << "[info] function1 done.\n";
}

void function2(const std::unique_ptr<Span>& parent) {
	auto span = addChildSpan(parent, "function2");

	cout << "[info] function2 done.\n";
}

void function3(const std::unique_ptr<Span>& parent) {
	auto span = addChildSpan(parent, "function3");

	cout << "[info] function3 done.\n";
}

void function4() {
	auto span = addSpan("function4");

	function2(span);
	function2(span);
	function2(span);
	function3(span);

	cout << "[info] function4 done.\n";
}

int main(int length, char** arguments) {
	int result = 0;
	if (length < 2) {
		cerr < "[error] Please specify a configuration file.\n";
		result = 1;
	}
	else {
		setUpTracer(arguments[1]);

		function1();
		function4();

		shutdownTracer();
	}

	return result;
}
