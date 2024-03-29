#include "stats2.h"

#include <string.h>


bool validLineCPU(const char* line) {
	return !strncmp(line, "cpu", 3);
}

void CoreStats::updateBuff() {
	this->reader.open(CORE_STATISTICS_SRC);
	reader.rdbuf()->sgetn(head, 4);
#if CORE_STATISTICS_STRICT_PARSING > 0
	if(validLineCPU(head) && !isdigit(head[3]))
#endif
	{
		this->main[REFERENCE] = std::move(this->main[IMMEDIATE]);
		for(size_t i = 0; i < ~State::TOTAL; i++) {
			reader >> this->main[IMMEDIATE][i];
		}
	}
	if(this->parse_all) {
		if(this->individual.size() == 0) {	// uninitialized
			while(reader.rdbuf()->sbumpc() != 10);
			while(reader.rdbuf()->sgetn(head, 4) && validLineCPU(head)) {
#if CORE_STATISTICS_STRICT_PARSING > 0
				if(head[3] != ' ' || isdigit(head[3]))
#endif
				{
					this->individual.emplace_back();
					for(size_t i = 0; i < ~State::TOTAL; i++) {
						reader >> this->individual.back()[IMMEDIATE][i];
					}
					while(reader.rdbuf()->sbumpc() != 10);
				}
			}
		} else {
			for(size_t core = 0; core < this->individual.size(); core++) {
				while(reader.rdbuf()->sbumpc() != 10);
				reader.rdbuf()->sgetn(head, 4);
#if CORE_STATISTICS_STRICT_PARSING > 0
				if(validLineCPU(head) && head[3] - '0' == core)
#endif
				{
					this->individual[core][REFERENCE] = std::move(this->individual[core][IMMEDIATE]);
					for(size_t i = 0; i < ~State::TOTAL; i++) {
						reader >> this->individual[core][IMMEDIATE][i];
					}
				}
			}
		}
	}
	this->reader.close();
}

float CoreStats::fromLast() {
	this->updateBuff();
	float active = (float)(this->getActive(IMMEDIATE) - this->getActive(REFERENCE));
	return active / (active + (float)(this->getIdle(IMMEDIATE) - this->getIdle(REFERENCE)));
}
float CoreStats::fromLastCore(uint16_t c) {
	if(this->validCore(c)) {
		this->updateBuff();
		float active = (float)(this->getCoreActive(c, IMMEDIATE) - this->getCoreActive(c, REFERENCE));
		return active / (active + (float)(this->getCoreIdle(c, IMMEDIATE) - this->getCoreIdle(c, REFERENCE)));
	}
	return 0.f;
}
void CoreStats::fromLastAll(std::vector<float>& out) {
	out.reserve(1 + this->individual.size());
	out[0] = this->fromLast();
	for(size_t i = 0; i < this->individual.size(); i++) {
		out[i + 1] = this->fromLastCore(i);
	}
}

float CoreStats::temp() {
	float value;
	std::ifstream file("/sys/class/thermal/thermal_zone0/temp");
	file >> value;
	file.close();
	return (value / 1000.f);
}
float CoreStats::temp_vcmd() {
	float val;
	FILE* fpipe = popen("vcgencmd measure_temp", "r");
	if(fscanf(fpipe, "temp=%f'C", &val) <= 0) { val = 0.f; }
	pclose(fpipe);
	return val;
}
float CoreStats::freq() {
	float value;
	std::ifstream file("/sys/devices/system/cpu/cpufreq/policy0/scaling_cur_freq");
	file >> value;
	file.close();
	return value * 1000.f;
}
float CoreStats::freq_vcmd() {
	unsigned int val;
	FILE* fpipe = popen("vcgencmd measure_clock arm", "r");
	if(fscanf(fpipe, "frequency(%*u)=%u", &val) <= 0) { val = 0; }
	pclose(fpipe);
	return (float)(val);
}
int CoreStats::throttlebits() {
	int val;
	FILE* fpipe = popen("vcgencmd get_throttled", "r");
	if(fscanf(fpipe, "throttled=%x", &val) <= 0) { val = 0; }
	pclose(fpipe);
	return val;
}
float CoreStats::volts() {
	float val;
	FILE* fpipe = popen("vcgencmd measure_volts core", "r");
	if(fscanf(fpipe, "volt=%fV", &val) <= 0) { val = 0.f; }
	pclose(fpipe);
	return val;
}