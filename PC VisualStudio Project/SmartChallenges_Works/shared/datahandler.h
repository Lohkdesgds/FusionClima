#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <sstream>
#include <limits>

#undef min
#undef max

constexpr double double_null = std::numeric_limits<double>::max();
constexpr uint16_t uint16_t_null = std::numeric_limits<uint16_t>::max();
constexpr uint32_t uint32_t_null = std::numeric_limits<uint32_t>::max();
constexpr int int_null = std::numeric_limits<int>::max();

constexpr size_t pred_size = 8;

struct line {
	uint16_t data[3] = { uint16_t_null, uint16_t_null, uint16_t_null }; // DIA/MES/ANO
	uint32_t hora_relatv = uint32_t_null; // offset [0..2400)
	double precipitacao_total_mm = double_null, __precipitacao_total_mm_cpy = double_null;
	double pressao_atm_estacao_mB = double_null;
	double pressao_atm_mar_mB = double_null;
	double pressao_atm_max_hr_ant = double_null;
	double pressao_atm_min_hr_ant = double_null;
	double radiacao_global_kj_p_ma2 = double_null;
	int temp_cpu = int_null;
	double temp_ar_bulbo_seco = double_null;
	double temp_ponto_orvalho = double_null;
	double temp_max_hora_ant = double_null;
	double temp_min_hora_ant = double_null;
	double temp_max_orvalho_ant = double_null;
	double temp_min_orvalho_ant = double_null;
	double tensao_bateria_estacao_v = double_null;
	int umidade_rel_max_hora_ant = int_null; // %
	int umidade_rel_min_hora_ant = int_null; // %
	int umidade_relativa_ar = int_null; // %
	int vento_ang_dg = int_null;
	double vento_rajada_max = double_null;
	double vento_vel_hora = double_null;

	bool interp(const std::string&);
	std::string export_str() const;

	void transform_chuva_bool_cut(int onegreateroreqthis);
};

struct pairing {
	double temp;
	double umid;
	double prec_chuva;
};

struct pairing8 {
	pairing mem[pred_size];

	// 0 == newest
	const pairing& get(size_t) const;
};

const size_t amount_of_opt = 22;

std::string remove_zeros(std::string);
std::string fill_spaces(std::string, size_t);

std::vector<line> readfile(const std::string&);
bool writefile(const std::string&, const std::vector<line>&);
bool writefile(const std::string&, const std::vector<pairing8>&);

bool validity_check_get(const std::vector<line>&, const size_t, pairing8&);

std::vector<pairing8> get_preds_steps(const std::vector<line>&, const double = 10.0);