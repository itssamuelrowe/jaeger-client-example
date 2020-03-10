#include <iostream>
#include <yaml-cpp/yaml.h>
#include <jaegertracing/Tracer.h>
#include <string>

void setUpTracer(const char* path) {
	auto file = YAML::LoadFile(path);
	auto config = jaegertracing::Config::parse(file);
	auto tracer = jaegertracing::Tracer::make("service", config, jaegertracing::logging::consoleLogger());
	opentracing::Tracer::InitGlobal(std::static_pointer_cast<opentracing::Tracer>(tracer));

	std::cout << "[info] The tracer was successfully initialized.\n";
}

void shutdownTracer() {
	auto globalTracer = opentracing::Tracer::Global();
	globalTracer->Close();

	std::cout << "[info] The tracer was successfully shutdown.\n";
}

std::unique_ptr<opentracing::Span> addSpan(const std::string& name) {
	auto globalTracer = opentracing::Tracer::Global();
	auto span = globalTracer->StartSpan(name);

	return span;
}

std::unique_ptr<opentracing::Span> addChildSpan(const std::unique_ptr<opentracing::Span>& parent, const std::string& name) {
	auto globalTracer = opentracing::Tracer::Global();
	auto parentContext = &parent->context();
	auto child = opentracing::ChildOf(parentContext);
	auto span = globalTracer->StartSpan(name, { child });

	std::cout << "[info] Successfully created tracer named '" << name << "'.\n";

	return span;
}

void function2(const std::unique_ptr<opentracing::Span>& parent) {
	auto span = addChildSpan(parent, "function2");

	std::cout << "[info] function2 done.\n";
}

void function3(const std::unique_ptr<opentracing::Span>& parent) {
	auto span = addChildSpan(parent, "function3");

	std::cout << "[info] function3 done.\n";
}

void function1() {
	auto span = addSpan("function1");

	function2(span);
	function3(span);

	std::cout << "[info] function1 done.\n";
}

void function4() {
	auto span = addSpan("function4");

	function2(span);
	function2(span);
	function2(span);
	function3(span);

	std::cout << "[info] function4 done.\n";
}

int main(int length, char** arguments) {
	int result = 0;
	if (length < 2) {
		std::cout << "[error] Please specify a configuration file.\n";
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
