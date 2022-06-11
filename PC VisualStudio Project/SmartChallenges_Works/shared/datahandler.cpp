#include "datahandler.h"

std::string remove_zeros(std::string in)
{
	if (in.find(',') == std::string::npos && in.find('.') == std::string::npos) return in;
	while (in.size() && in.back() == '0') in.pop_back();
	if (in.size() && (in.back() == ',' || in.back() == '.')) in.pop_back();
	return in;
}

std::string fill_spaces(std::string in, size_t min)
{
	while (in.size() < min) in += ' ';
	return in;
}

//line::line(const line& oth)
//{
//	memcpy_s(this, sizeof(line), &oth, sizeof(oth));
//}
//
//line::line(line&& oth) noexcept
//{
//	memcpy_s(this, sizeof(line), &oth, sizeof(oth));
//}
//
//line& line::operator=(const line& oth)
//{
//	memcpy_s(this, sizeof(line), &oth, sizeof(oth));
//	return *this;
//}
//
//void line::operator=(line&& oth) noexcept
//{
//	memcpy_s(this, sizeof(line), &oth, sizeof(oth));
//}

bool line::interp(const std::string& buf)
{
	std::stringstream ss(buf + ';');
	std::string buf2;
	bool selected_failed = false;

	for (size_t each = 0; each < amount_of_opt; ++each)
	{
		if (!std::getline(ss, buf2, ';')) {
			std::cout << "Error: invalid amount of columns!" << std::endl;
			return false;
		}

		switch (each) {
		case 0:
			//uint16_t data[3]; // DIA/MES/ANO
			sscanf_s(buf2.c_str(), "%hi/%hi/%hi", &data[0], &data[1], &data[2]);
			break;
		case 1:
			//uint32_t hora_relatv; // offset [0..2400)
			sscanf_s(buf2.c_str(), "%d", &hora_relatv);
			break;
		case 2:
			//double precipitacao_total_mm;
			sscanf_s(buf2.c_str(), "%lf", &precipitacao_total_mm);
			__precipitacao_total_mm_cpy = precipitacao_total_mm;
			selected_failed |= (precipitacao_total_mm > 500.0 || precipitacao_total_mm < 0.0);
			break;
		case 3:
			//double pressao_atm_estacao_mB;
			sscanf_s(buf2.c_str(), "%lf", &pressao_atm_estacao_mB);
			break;
		case 4:
			//double pressao_atm_mar_mB;
			sscanf_s(buf2.c_str(), "%lf", &pressao_atm_mar_mB);
			break;
		case 5:
			//double pressao_atm_max_hr_ant;
			sscanf_s(buf2.c_str(), "%lf", &pressao_atm_max_hr_ant);
			break;
		case 6:
			//double pressao_atm_min_hr_ant;
			sscanf_s(buf2.c_str(), "%lf", &pressao_atm_min_hr_ant);
			break;
		case 7:
			//double radiacao_global_kj_p_ma2;
			sscanf_s(buf2.c_str(), "%lf", &radiacao_global_kj_p_ma2);
			break;
		case 8:
			//int temp_cpu;
			sscanf_s(buf2.c_str(), "%i", &temp_cpu);
			break;
		case 9:
			//double temp_ar_bulbo_seco;
			sscanf_s(buf2.c_str(), "%lf", &temp_ar_bulbo_seco);
			selected_failed |= (temp_ar_bulbo_seco < -100.0 || temp_ar_bulbo_seco > 100.0);
			break;
		case 10:
			//double temp_ponto_orvalho;
			sscanf_s(buf2.c_str(), "%lf", &temp_ponto_orvalho);
			break;
		case 11:
			//double temp_max_hora_ant;
			sscanf_s(buf2.c_str(), "%lf", &temp_max_hora_ant);
			break;
		case 12:
			//double temp_min_hora_ant;
			sscanf_s(buf2.c_str(), "%lf", &temp_min_hora_ant);
			break;
		case 13:
			//double temp_max_orvalho_ant;
			sscanf_s(buf2.c_str(), "%lf", &temp_max_orvalho_ant);
			break;
		case 14:
			//double temp_min_orvalho_ant;
			sscanf_s(buf2.c_str(), "%lf", &temp_min_orvalho_ant);
			break;
		case 15:
			//double tensao_bateria_estacao_v;
			sscanf_s(buf2.c_str(), "%lf", &tensao_bateria_estacao_v);
			break;
		case 16:
			//int umidade_rel_max_hora_ant; // %
			sscanf_s(buf2.c_str(), "%i", &umidade_rel_max_hora_ant);
			break;
		case 17:
			//int umidade_rel_min_hora_ant; // %
			sscanf_s(buf2.c_str(), "%i", &umidade_rel_min_hora_ant);
			break;
		case 18:
			//int umidade_relativa_ar; // %
			sscanf_s(buf2.c_str(), "%i", &umidade_relativa_ar);
			selected_failed |= (umidade_relativa_ar < 0.0 || umidade_relativa_ar > 110.0);
			break;
		case 19:
			//int vento_ang_dg;
			sscanf_s(buf2.c_str(), "%i", &vento_ang_dg);
			break;
		case 20:
			//double vento_rajada_max;
			sscanf_s(buf2.c_str(), "%lf", &vento_rajada_max);
			break;
		case 21:
			//double vento_vel_hora;
			sscanf_s(buf2.c_str(), "%lf", &vento_vel_hora);
			break;
		default:
			std::cout << "Too many cases? ERROR!";
			return false;
		}
	}
	return !selected_failed;
}

std::string line::export_str() const
{
	std::string str;
	const auto len = snprintf(NULL, 0, "%02hu/%02hu/%04hu;", data[0], data[1], data[2]);
	if (len <= 0) throw std::runtime_error("Can't export");
	str.resize(static_cast<size_t>(len) + 1, '\0');
	snprintf(str.data(), str.size(), "%02hu/%02hu/%04hu;", data[0], data[1], data[2]);
	if (str.size() && str.back() == '\0') str.pop_back();
	str +=
		(hora_relatv == uint32_t_null ? "null" : remove_zeros(std::to_string(hora_relatv))) + ";" +
		(precipitacao_total_mm == double_null ? "null" : remove_zeros(std::to_string(precipitacao_total_mm))) + ";" +
		(pressao_atm_estacao_mB == double_null ? "null" : remove_zeros(std::to_string(pressao_atm_estacao_mB))) + ";" +
		(pressao_atm_mar_mB == double_null ? "null" : remove_zeros(std::to_string(pressao_atm_mar_mB))) + ";" +
		(pressao_atm_max_hr_ant == double_null ? "null" : remove_zeros(std::to_string(pressao_atm_max_hr_ant))) + ";" +
		(pressao_atm_min_hr_ant == double_null ? "null" : remove_zeros(std::to_string(pressao_atm_min_hr_ant))) + ";" +
		(radiacao_global_kj_p_ma2 == double_null ? "null" : remove_zeros(std::to_string(radiacao_global_kj_p_ma2))) + ";" +
		(temp_cpu == int_null ? "null" : remove_zeros(std::to_string(temp_cpu))) + ";" +
		(temp_ar_bulbo_seco == double_null ? "null" : remove_zeros(std::to_string(temp_ar_bulbo_seco))) + ";" +
		(temp_ponto_orvalho == double_null ? "null" : remove_zeros(std::to_string(temp_ponto_orvalho))) + ";" +
		(temp_max_hora_ant == double_null ? "null" : remove_zeros(std::to_string(temp_max_hora_ant))) + ";" +
		(temp_min_hora_ant == double_null ? "null" : remove_zeros(std::to_string(temp_min_hora_ant))) + ";" +
		(temp_max_orvalho_ant == double_null ? "null" : remove_zeros(std::to_string(temp_max_orvalho_ant))) + ";" +
		(temp_min_orvalho_ant == double_null ? "null" : remove_zeros(std::to_string(temp_min_orvalho_ant))) + ";" +
		(tensao_bateria_estacao_v == double_null ? "null" : remove_zeros(std::to_string(tensao_bateria_estacao_v))) + ";" +
		(umidade_rel_max_hora_ant == int_null ? "null" : remove_zeros(std::to_string(umidade_rel_max_hora_ant))) + ";" +
		(umidade_rel_min_hora_ant == int_null ? "null" : remove_zeros(std::to_string(umidade_rel_min_hora_ant))) + ";" +
		(umidade_relativa_ar == int_null ? "null" : remove_zeros(std::to_string(umidade_relativa_ar))) + ";" +
		(vento_ang_dg == int_null ? "null" : remove_zeros(std::to_string(vento_ang_dg))) + ";" +
		(vento_rajada_max == double_null ? "null" : remove_zeros(std::to_string(vento_rajada_max))) + ";" +
		(vento_vel_hora == double_null ? "null" : remove_zeros(std::to_string(vento_vel_hora)));
	for (auto& i : str) if (i == '.') i = ',';
	return str;
}

void line::transform_chuva_bool_cut(int onegreateroreqthis)
{
	precipitacao_total_mm = (precipitacao_total_mm >= onegreateroreqthis && precipitacao_total_mm != double_null) ? 1 : 0;
}
// 0 == newest
const pairing& pairing8::get(size_t p) const
{
	p = p >= pred_size ? (pred_size - 1) : p;
	return mem[pred_size - p - 1];
}

std::vector<line> readfile(const std::string& path)
{
	std::fstream fp(path, std::ios::in | std::ios::binary);
	if (!fp) return {};

	std::vector<line> lst;
	std::string buf;
	bool possible_end = false;
	size_t itc = 0;
	size_t fail_count = 0;

	while (fp && !fp.eof())
	{
		std::getline(fp, buf, '\n');
		for (auto& i : buf) if (i == ',') i = '.';
		while (buf.size() && (buf.back() == '\r' || buf.back() == '\n')) buf.pop_back();
		if (buf.empty()) {
			if (possible_end) return lst;
			else possible_end = true;
			continue;
		}
		possible_end = false;

		line lin;
		if (lin.interp(buf)) {
			lst.emplace_back(std::move(lin));
		}
		else {
			++fail_count;
			if (buf.find("null") == std::string::npos) std::cout << "Warn: Failed N" << fail_count << " (invalid) block #" << itc << ": " << buf << std::endl;
		}

		++itc;
	}

	if (fail_count) std::cout << "Warn: Got null count: " << fail_count << std::endl;
	else std::cout << "Info: Got no null" << std::endl;

	return lst;
}

bool writefile(const std::string& path, const std::vector<line>& vec)
{
	std::fstream fo(path, std::ios::out | std::ios::binary);
	if (!fo) return false;

	for (const auto& i : vec) {
		fo << i.export_str() << '\n';
	}
	
	return fo.good();
}

bool writefile(const std::string& path, const std::vector<pairing8>& vec)
{
	std::fstream fo(path, std::ios::out | std::ios::binary);
	if (!fo) return false;

	for (size_t p = 0; p < vec.size(); ++p) {
		fo << p << ":\n";
		for (size_t u = 0; u < pred_size; ++u) {
			fo << vec[p].mem[u].temp << " ";
			fo << vec[p].mem[u].umid << " ";
			fo << vec[p].mem[u].prec_chuva << "\n";
		}
	}
	
	return fo.good();
}

bool validity_check_get(const std::vector<line>& thingys, const size_t pp, pairing8& a)
{
	if (pp < pred_size) return false;
	for (size_t k = 0; k < pred_size; ++k) {
		a.mem[k].temp = thingys[pp - ((pred_size - k) - 1)].temp_ar_bulbo_seco;
		a.mem[k].umid = thingys[pp - ((pred_size - k) - 1)].umidade_relativa_ar;
		a.mem[k].prec_chuva = thingys[pp - ((pred_size - k) - 1)].__precipitacao_total_mm_cpy;
	}
	for (size_t k = 0; k < pred_size; ++k)
	{
		const auto u = pp - ((pred_size - k) - 1);
		if (
			(thingys[u].precipitacao_total_mm > 1000.0 || thingys[u].precipitacao_total_mm < 0.0) ||
			(thingys[u].umidade_relativa_ar < 0.0 || thingys[u].umidade_relativa_ar > 100.0) ||
			(thingys[u].temp_ar_bulbo_seco < -100.0 || thingys[u].temp_ar_bulbo_seco > 200.0)) return false;
	}
	return true;
}

std::vector<pairing8> get_preds_steps(const std::vector<line>& thingys, const double minchuva)
{
	std::vector<pairing8> ts;

	for (size_t p = 7; p < thingys.size(); ++p) {
		if (thingys[p].precipitacao_total_mm >= minchuva) {
			pairing8 a{};
			if (validity_check_get(thingys, p, a))
			{
				ts.push_back(a);
			}
			else {
				std::cout << "Skipped #" << p << " (discarded/invalid).\n";
			}
		}
	}
	return ts;
}