// #include <iostream>
// #include <vector>
// #include <string>
// #include <numeric>
// #include <cstring>
// #include <sstream>
// #include <algorithm>
// #include <zlib.h>
#include "sclib.h"


/*!
 * Welcome text.
 */
void sc::hello() {
  std::cout << std::endl;
  std::cout << "__ Welcome to \033[1;32mStream\033[0m\033[1;34mCraft\033[0m __" << std::endl;
}


/*!
 * Encodes to little endian binary a vector of doubles based on a precision integer.
 */
std::string sc::encode_little_endian(const std::vector<double>& input, const int& precision) {

  if (precision == 8) {
    std::vector<uint8_t> bytes(sizeof(double) * input.size());
    std::memcpy(bytes.data(), input.data(), bytes.size());
    std::string result(bytes.begin(), bytes.end());
    return result;

  } else if (precision == 4) {
    std::vector<uint8_t> bytes(sizeof(float) * input.size());
    for (size_t i = 0; i < input.size(); ++i) {
        float floatValue = static_cast<float>(input[i]);
        std::memcpy(bytes.data() + i * sizeof(float), &floatValue, sizeof(float));
    }
    std::string result(bytes.begin(), bytes.end());
    return result;

  } else {
    throw("Precision must be 4 (32-bit) or 8 (64-bit)!");
  }
}


/*!
 * Decodes from a little endian binary string to a vector of doubles according a precision integer.
 */
std::vector<double> sc::decode_little_endian(const std::string& str, const int& precision) {

  std::vector<unsigned char> bytes(str.begin(), str.end());
  int bytes_size = (bytes.size() / precision);
  std::vector<double> result(bytes_size);

  for (int i = 0; i < bytes_size; ++i) {

    if (precision == 8) {
      result[i] = reinterpret_cast<double&>(bytes[i * precision]);

    } else if (precision == 4) {
      float floatValue;
      std::memcpy(&floatValue, &bytes[i * precision], sizeof(float));
      result[i] = static_cast<double>(floatValue);

    } else {
      throw("Precision must be 4 (32-bit) or 8 (64-bit)!");
    }
  }

  return result;
}


/*!
 * Compresses a string using the zlib library (https://zlib.net/).
 */
std::string sc::compress_zlib(const std::string& str) {

  std::vector<char> compressed_data;

  z_stream zs;

  memset(&zs, 0, sizeof(zs));

  if (deflateInit(&zs, Z_DEFAULT_COMPRESSION) != Z_OK) {
    throw std::runtime_error("deflateInit failed while initializing zlib for compression");
  }

  zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(str.data()));
  zs.avail_in = str.size();

  int ret;
  char outbuffer[32768];

  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = sizeof(outbuffer);

    ret = deflate(&zs, Z_FINISH);

    if (compressed_data.size() < zs.total_out) {
      compressed_data.insert(compressed_data.end(), outbuffer, outbuffer + (zs.total_out - compressed_data.size()));
    }

  } while (ret == Z_OK);

  deflateEnd(&zs);

  return std::string(compressed_data.begin(), compressed_data.end());
}


/*!
 * Decompresses a string using the zlib library (https://zlib.net/).
 */
std::string sc::decompress_zlib(const std::string& compressed_string) {

  z_stream zs;

  memset(&zs, 0, sizeof(zs));

  inflateInit(&zs);

  zs.next_in = (Bytef*)compressed_string.data();

  zs.avail_in = compressed_string.size();

  int ret;

  char outbuffer[32768];

  std::string outstring;

  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);

    zs.avail_out = sizeof(outbuffer);

    ret = inflate(&zs, 0);

    if (outstring.size() < zs.total_out) {
      outstring.append(outbuffer, zs.total_out - outstring.size());
    }
  }

  while (ret == Z_OK);

  inflateEnd(&zs);

  return outstring;
}


/*!
 * Encodes a string with binary data to a Base64 string.
 */
std::string sc::encode_base64(const std::string& str) {

  static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  std::string encoded_data;

  encoded_data.reserve(((str.size() + 2) / 3) * 4);

  for (size_t i = 0; i < str.size(); i += 3) {

    int b = (str[i] & 0xFC) >> 2;
    encoded_data.push_back(base64_chars[b]);

    if (i + 1 < str.size()) {
      b = ((str[i] & 0x03) << 4) | ((str[i + 1] & 0xF0) >> 4);
      encoded_data.push_back(base64_chars[b]);

      if (i + 2 < str.size()) {
        b = ((str[i + 1] & 0x0F) << 2) | ((str[i + 2] & 0xC0) >> 6);
        encoded_data.push_back(base64_chars[b]);
        b = str[i + 2] & 0x3F;
        encoded_data.push_back(base64_chars[b]);

      } else {
        b = (str[i + 1] & 0x0F) << 2;
        encoded_data.push_back(base64_chars[b]);
        encoded_data.push_back('=');
      }

    } else {
      b = (str[i] & 0x03) << 4;
      encoded_data.push_back(base64_chars[b]);
      encoded_data.push_back('=');
      encoded_data.push_back('=');
    }
  }

  return encoded_data;
}



/*!
 * Decodes a Base64 string into a string with binary data.
 */
std::string sc::decode_base64(const std::string& encoded_string) {

  std::string decoded_string;

  decoded_string.reserve((encoded_string.size() * 3) / 4);

  int val = 0;
  int valb = -8;
  for (char c : encoded_string) {
    if (c == '=') {
      valb -= 6;
      continue;
    }
    if (c >= 'A' && c <= 'Z') {
      c -= 'A';
    } else if (c >= 'a' && c <= 'z') {
      c -= 'a' - 26;
    } else if (c >= '0' && c <= '9') {
      c -= '0' - 52;
    } else if (c == '+') {
      c = 62;
    } else if (c == '/') {
      c = 63;
    } else {
      continue;
    }
    val = (val << 6) + c;
    valb += 6;
    if (valb >= 0) {
      decoded_string.push_back(char((val >> valb) & 0xFF));
      valb -= 8;
    }
  }

  return decoded_string;
}



/*!
 * Extracts headers from spectrum elements of an mzML file according to a vector of indices,
   indicating the location of the spectra to retrieve metadata.
   This is a private function of the class MZML in sc namespace.
 */
sc::MZML_SPECTRA_HEADERS sc::MZML::extract_mzml_spectra_headers(const std::vector<int>& idxs) {

  std::string search_run = "//run";

  pugi::xpath_node xps_run = root.select_node(search_run.c_str());

  pugi::xml_node spec_list = xps_run.node().child("spectrumList");

  sc::MZML_SPECTRA_HEADERS headers;

  int number_spectra = idxs.size();

  if (number_spectra == 0) {
    std::cerr << "Warning: No indices given!" << std::endl;
    return headers;
  }

  if (spec_list) {

    std::vector<pugi::xml_node> spectra;

    for (pugi::xml_node child = spec_list.first_child(); child; child = child.next_sibling()) {
      spectra.push_back(child);
    }

    headers.spec_index.resize(number_spectra);
    headers.spec_id.resize(number_spectra);
    headers.spec_scan.resize(number_spectra);
    headers.spec_array_length.resize(number_spectra);
    headers.spec_level.resize(number_spectra);
    headers.spec_mode.resize(number_spectra);
    headers.spec_polarity.resize(number_spectra);
    headers.spec_lowmz.resize(number_spectra);
    headers.spec_highmz.resize(number_spectra);
    headers.spec_bpmz.resize(number_spectra);
    headers.spec_bpint.resize(number_spectra);
    headers.spec_tic.resize(number_spectra);
    headers.spec_title.resize(number_spectra);
    headers.scan_rt.resize(number_spectra);
    headers.scan_drift.resize(number_spectra);
    headers.scan_filter_string.resize(number_spectra);
    headers.scan_config.resize(number_spectra);
    headers.scan_injection_ion_time.resize(number_spectra);
    headers.precursor_scan.resize(number_spectra);
    headers.window_mz.resize(number_spectra);
    headers.window_mzlow.resize(number_spectra);
    headers.window_mzhigh.resize(number_spectra);
    headers.ion_mz.resize(number_spectra);
    headers.ion_intensity.resize(number_spectra);
    headers.ion_charge.resize(number_spectra);
    headers.activation_type.resize(number_spectra);
    headers.activation_ce.resize(number_spectra);

    for (int i = 0; i < number_spectra; i++) {

      const int& index = idxs[i];

      const pugi::xml_node& spec = spectra[index];

      headers.spec_index[i] = spec.attribute("index").as_int();

      std::string spec_id = spec.attribute("id").as_string();

      headers.spec_id[i] = spec_id;

      std::size_t poslastEqual = spec_id.rfind('=');

      headers.spec_scan[i] = std::stoi(spec_id.substr(poslastEqual + 1));

      headers.spec_array_length[i] = spec.attribute("defaultArrayLength").as_int();

      pugi::xml_node level_node = spec.find_child_by_attribute("cvParam", "name", "ms level");
      headers.spec_level[i] = level_node.attribute("value").as_int();

      pugi::xml_node centroid_node = spec.find_child_by_attribute("cvParam", "accession", "MS:1000127");
      pugi::xml_node profile_node = spec.find_child_by_attribute("cvParam", "accession", "MS:1000128");

      if (centroid_node) {
        headers.spec_mode[i] = "centroid";
      } else if (profile_node) {
        headers.spec_mode[i] = "profile";
      } else {
        headers.spec_mode[i] = "";
      }

      pugi::xml_node pol_pos_node = spec.find_child_by_attribute("cvParam", "accession", "MS:1000130");
      pugi::xml_node pol_neg_node = spec.find_child_by_attribute("cvParam", "accession", "MS:1000129");

      if (pol_pos_node) {
        headers.spec_polarity[i] = "positive";
      } else if (pol_neg_node) {
        headers.spec_polarity[i] = "negative";
      } else {
        headers.spec_polarity[i] = "";
      }

      pugi::xml_node lowmz_node = spec.find_child_by_attribute("cvParam", "name", "lowest observed m/z");
      headers.spec_lowmz[i] = lowmz_node.attribute("value").as_double();

      pugi::xml_node highmz_node = spec.find_child_by_attribute("cvParam", "name", "highest observed m/z");
      headers.spec_highmz[i] = highmz_node.attribute("value").as_double();

      pugi::xml_node bpmz_node = spec.find_child_by_attribute("cvParam", "name", "base peak m/z");
      headers.spec_bpmz[i] = bpmz_node.attribute("value").as_double();

      pugi::xml_node bpint_node = spec.find_child_by_attribute("cvParam", "name", "base peak intensity");
      headers.spec_bpint[i] = bpint_node.attribute("value").as_double();

      pugi::xml_node tic_node = spec.find_child_by_attribute("cvParam", "name", "total ion current");
      headers.spec_tic[i] = tic_node.attribute("value").as_double();

      pugi::xml_node title_node = spec.find_child_by_attribute("cvParam", "name", "spectrum title");
      headers.spec_title[i] = title_node.attribute("value").as_string();

      pugi::xml_node node_scan = spec.child("scanList").child("scan");

      pugi::xml_node rt_node = node_scan.find_child_by_attribute("cvParam", "name", "scan start time");
      std::string rt_unit = rt_node.attribute("unitName").as_string();
      double rt_val = rt_node.attribute("value").as_double();
      if (rt_unit == "minute") rt_val = rt_val * 60;
      headers.scan_rt[i] = rt_val;

      pugi::xml_node drift_node = node_scan.find_child_by_attribute("cvParam", "name", "ion mobility drift time");
      headers.scan_drift[i] = drift_node.attribute("value").as_double();

      pugi::xml_node filter_string_node = node_scan.find_child_by_attribute("cvParam", "name", "filter string");
      headers.scan_filter_string[i] = filter_string_node.attribute("value").as_string();

      pugi::xml_node config_node = node_scan.find_child_by_attribute("cvParam", "name", "preset scan configuration");
      headers.scan_config[i] = config_node.attribute("value").as_int();

      pugi::xml_node iit_node = node_scan.find_child_by_attribute("cvParam", "name", "ion injection time");
      headers.scan_injection_ion_time[i] = iit_node.attribute("value").as_double();

      pugi::xml_node precursor = spec.child("precursorList").child("precursor");

      if (precursor) {

        std::string pre_scan_str = precursor.attribute("spectrumRef").as_string();

        if (pre_scan_str != "") {
          std::size_t poslastEqual = pre_scan_str.rfind('=');
          int pre_scan_n = std::stoi(pre_scan_str.substr(poslastEqual + 1));
          headers.precursor_scan[i] = pre_scan_n;
        }

        pugi::xml_node isolation = precursor.child("isolationWindow");

        pugi::xml_node window_mz_node = isolation.find_child_by_attribute("cvParam", "name", "isolation window target m/z");
        headers.window_mz[i] = window_mz_node.attribute("value").as_double();

        pugi::xml_node window_mzlow_node = isolation.find_child_by_attribute("cvParam", "name", "isolation window lower offset");
        headers.window_mzlow[i] = window_mzlow_node.attribute("value").as_double();

        pugi::xml_node window_mzhigh_node = isolation.find_child_by_attribute("cvParam", "name", "isolation window upper offset");
        headers.window_mzhigh[i] = window_mzhigh_node.attribute("value").as_double();

        pugi::xml_node slected_ion = precursor.child("selectedIonList").first_child();

        if (slected_ion) {

          pugi::xml_node ion_mz_node = slected_ion.find_child_by_attribute("cvParam", "name", "selected ion m/z");
          headers.ion_mz[i] = ion_mz_node.attribute("value").as_double();

          pugi::xml_node ion_intensity_node = slected_ion.find_child_by_attribute("cvParam", "name", "peak intensity");
          headers.ion_intensity[i] = ion_intensity_node.attribute("value").as_double();

          pugi::xml_node ion_charge_node = slected_ion.find_child_by_attribute("cvParam", "name", "charge state");
          headers.ion_charge[i] = ion_charge_node.attribute("value").as_int();

        }

        pugi::xml_node activation = precursor.child("activation");

        if (activation) {

          pugi::xml_node activation_type_node = activation.first_child();
          headers.activation_type[i] = activation_type_node.name();

          pugi::xml_node activation_ce_node = activation.find_child_by_attribute("cvParam", "name", "collision energy");
          headers.activation_ce[i] = activation_ce_node.attribute("value").as_double();

        }
      }
    } // end of i loop

  } else {
    std::cerr << "Warning: Spectra list not found in the mzML file!" << std::endl;
  }

  return headers;
}



/*!
 * Extracts metadata from a binary node of an mzML file.
   This is a private function of the class MZML in sc namespace.
 */
sc::MZML_BINARY_METADATA sc::MZML::extract_mzml_binary_metadata(const pugi::xml_node& bin) {

  MZML_BINARY_METADATA mtd;

  std::string precision_str;
  int precision_int;

  pugi::xml_node node_float_32 = bin.find_child_by_attribute("cvParam", "accession", "MS:1000521");
  pugi::xml_node node_integer = bin.find_child_by_attribute("cvParam", "accession", "MS:1000522");
  pugi::xml_node node_float_64 = bin.find_child_by_attribute("cvParam", "accession", "MS:1000523");

  if (node_float_64) {
    precision_str = node_float_64.attribute("name").as_string();
  } else if (node_float_32) {
    precision_str = node_float_32.attribute("name").as_string();
  } else if (node_integer){
    precision_str = node_integer.attribute("name").as_string();
  } else {
    throw("Encoding precision with accession MS:1000521, MS:1000522 or MS:1000523 not found!");
  }

  std::sscanf(precision_str.c_str(), "%d%*c", &precision_int);

  precision_int = precision_int / 8;

  mtd.precision = precision_int;

  if (node_float_64) {
    mtd.type = "double";

  } else if (node_integer != NULL) {
    mtd.type = "integer";

  } else if (node_float_32 != NULL) {
    mtd.type = "float";

  } else {
    throw("Encoding precision with accession MS:1000521, MS:1000522 or MS:1000523 not found!");
  }

  std::string compression = "none";

  pugi::xml_node node_comp = bin.find_child_by_attribute("cvParam", "accession", "MS:1000574");

  if (node_comp) {
    compression = node_comp.attribute("name").as_string();

    if (compression == "zlib" || compression == "zlib compression") {
      compression = "gzip";
    }
  }

  mtd.compression = compression;

  pugi::xml_node node_mz = bin.find_child_by_attribute("cvParam", "accession", "MS:1000514");
  pugi::xml_node node_int = bin.find_child_by_attribute("cvParam", "accession", "MS:1000515");
  pugi::xml_node node_rt = bin.find_child_by_attribute("cvParam", "accession", "MS:1000595");
  pugi::xml_node node_other = bin.find_child_by_attribute("cvParam", "accession", "MS:1000786");

  if (node_mz) {
    mtd.name = "mz";

  } else if (node_int) {
    mtd.name = "intensity";

  } else if (node_rt != NULL) {
    mtd.name = "rt";

  } else if (node_other) {
    std::string name = node_other.attribute("value").as_string();
    name.erase(std::remove(name.begin(), name.end(), ' '), name.end());
    mtd.name = name;

  } else {
    mtd.name = "val_";
  }

  return mtd;
}



/*!
 * Extracts metadata from each binary array in the first node of the spectra list of an mzML file.
   This is a private function of the class MZML in sc namespace. Updates the private fields named binary_*.
 */
void sc::MZML::get_mzml_spectra_binary_metadata(const pugi::xml_node& first_node) {

  pugi::xml_node binary_list = first_node.child("binaryDataArrayList");

  int counter = 0;

  for (const pugi::xml_node& bin: binary_list.children("binaryDataArray")) {

    MZML_BINARY_METADATA mtd;

    mtd = extract_mzml_binary_metadata(bin);

    if (mtd.name == "val_") {
      mtd.name = "val_" + std::to_string(counter);
    }

    spectra_binary_metadata.push_back(mtd);

    counter++;

  }

  spectra_binary_arrays_number = counter;
}



/*!
 * Extracts a spectrum from a binary array list node from an mzML file.
   This is a private function of the class MZML in sc namespace.
 */
std::vector<std::vector<double>> sc::MZML::extract_mzml_spectrum(const pugi::xml_node& spectrum_node) {

  std::vector<std::vector<double>> spectrum;

  int number_traces = spectrum_node.attribute("defaultArrayLength").as_int();

  pugi::xml_node node_binary_list = spectrum_node.child("binaryDataArrayList");

  int number_bins = node_binary_list.attribute("count").as_int();

  if (spectra_binary_arrays_number != number_bins) {
    throw("Binary array length does not match binary array length of first spectrum!");
  }

  spectrum.resize(number_bins);

  int counter = 0;

  for (auto i = node_binary_list.children("binaryDataArray").begin(); i != node_binary_list.children("binaryDataArray").end(); ++i) {

    const pugi::xml_node& bin = *i;

    pugi::xml_node node_binary = bin.child("binary");

    std::string encoded_string = node_binary.child_value();

    std::string decoded_string = decode_base64(encoded_string);

    if (spectra_binary_metadata[counter].compression == "gzip") {
      decoded_string = sc::decompress_zlib(decoded_string);
    }

    spectrum[counter] = sc::decode_little_endian(decoded_string, spectra_binary_metadata[counter].precision);

    int bin_array_size = spectrum[counter].size();

    if (bin_array_size != number_traces) {
      throw("Number of traces in binary array does not match the value of the spectrum header!");
    }

    if (spectra_binary_metadata[counter].name == "rt") {
      pugi::xml_node node_unit = bin.find_child_by_attribute("cvParam", "unitCvRef", "UO");
      std::string unit = node_unit.attribute("unitName").as_string();

      if (unit == "minute") {
        for (double &j : spectrum[counter]) {
          j *= 60;
        }
      }
    }

    counter++;
  }

  return spectrum;
}



/*!
 * Extracts spectra from an mzML file according to a vector of indices,
   indicating the location of the spectra to retrieve.
   This is a private function of the class MZML in sc namespace.
 */
std::vector<std::vector<std::vector<double>>> sc::MZML::extract_mzml_spectra(const std::vector<int>& idxs) {

  std::string search_run = "//run";

  pugi::xpath_node xps_run = root.select_node(search_run.c_str());

  pugi::xml_node spec_list = xps_run.node().child("spectrumList");

  std::vector<std::vector<std::vector<double>>> sp;

  int number_spectra_to_extract = idxs.size();

  if (number_spectra_to_extract == 0) {
    std::cerr << "Warning: No indices given!" << std::endl;
    return sp;
  }

  if (spec_list) {

    sp.resize(number_spectra_to_extract);

    std::vector<pugi::xml_node> spectra;

    for (pugi::xml_node child = spec_list.first_child(); child; child = child.next_sibling()) {
      spectra.push_back(child);
    }

    for (int i = 0; i < number_spectra_to_extract; i++) {

      const int& index = idxs[i];

      const pugi::xml_node& spectrum_node = spectra[index];

      sp[i] = extract_mzml_spectrum(spectrum_node);
    }
  }

  return sp;
}



/*!
 * Extracts headers from chromatograms elements of an mzML file according to a vector of indices,
   indicating the location of the spectra to retrieve metadata.
   This is a private function of the class MZML in sc namespace.
 */
sc::MZML_CHROMATOGRAMS_HEADERS sc::MZML::extract_mzml_chrom_headers(const std::vector<int>& idxs) {

  std::string search_run = "//run";

  pugi::xpath_node xps_run = root.select_node(search_run.c_str());

  pugi::xml_node chrom_list = xps_run.node().child("chromatogramList");

  sc::MZML_CHROMATOGRAMS_HEADERS headers;

  int number_chroms = idxs.size();

  if (number_chroms == 0) {
    std::cerr << "Warning: No indices given!" << std::endl;
    return headers;
  }

  if (chrom_list) {

    std::vector<pugi::xml_node> chroms;

    for (pugi::xml_node child = chrom_list.first_child(); child; child = child.next_sibling()) {
      chroms.push_back(child);
    }

    if (chroms.size() == 0) {
      std::cerr << "No chromatograms found!" << std::endl;
      return headers;
    }

    headers.chrom_index.resize(number_chroms);
    headers.chrom_id.resize(number_chroms);
    headers.chrom_array_length.resize(number_chroms);
    headers.chrom_polarity.resize(number_chroms);
    headers.precursor_mz.resize(number_chroms);
    headers.activation_type.resize(number_chroms);
    headers.activation_ce.resize(number_chroms);
    headers.product_mz.resize(number_chroms);

    for (int i = 0; i < number_chroms; i++) {

      const int& index = idxs[i];

      const pugi::xml_node& chrom = chroms[index];

      headers.chrom_id[i] = chrom.attribute("index").as_int();

      std::string id = chrom.attribute("id").as_string();

      headers.chrom_id[i] = id;

      headers.chrom_array_length[i] = chrom.attribute("defaultArrayLength").as_int();

      pugi::xml_node pol_pos_node = chrom.find_child_by_attribute("cvParam", "accession", "MS:1000130");
      pugi::xml_node pol_neg_node = chrom.find_child_by_attribute("cvParam", "accession", "MS:1000129");

      if (pol_pos_node) {
        headers.chrom_polarity[i] = "positive";

      } else if (pol_neg_node) {
        headers.chrom_polarity[i] = "negative";
      } else {
        headers.chrom_polarity[i] = "";
      }

      pugi::xml_node precursor = chrom.child("precursor");

      if (precursor != NULL) {

        pugi::xml_node isolation = precursor.child("isolationWindow");

        pugi::xml_node pre_mz_node = isolation.find_child_by_attribute("cvParam", "name", "isolation window target m/z");

        headers.precursor_mz[i] = pre_mz_node.attribute("value").as_double();

        pugi::xml_node activation = precursor.child("activation");

        if (activation) {

          pugi::xml_node activation_type_node = activation.first_child();
          headers.activation_type[i] = activation_type_node.name();

          pugi::xml_node activation_ce_node = activation.find_child_by_attribute("cvParam", "name", "collision energy");
          headers.activation_ce[i] = activation_ce_node.attribute("value").as_double();

        }
      }

      pugi::xml_node product = chrom.child("product");

      if (product) {

        pugi::xml_node isolation = product.child("isolationWindow");

        pugi::xml_node pro_mz_node = isolation.find_child_by_attribute("cvParam", "name", "isolation window target m/z");
        headers.product_mz[i] = pro_mz_node.attribute("value").as_double();

      }
    } // end of i loop

  } else {
    std::cerr << "Warning: Chromatogram list not found in the mzML file!" << std::endl;
  }

  return headers;
}



/*!
 * Extracts a chromatograms from a binary array list node from an mzML file.
   This is a private function of the class MZML in sc namespace.
 */
std::vector<std::vector<double>> sc::MZML::extract_mzml_chromatogram(const pugi::xml_node& chrom_node) {

  std::vector<std::vector<double>> chrom;

  int number_traces = chrom_node.attribute("defaultArrayLength").as_int();

  pugi::xml_node node_binary_list = chrom_node.child("binaryDataArrayList");

  int number_bins = node_binary_list.attribute("count").as_int();

  chrom.resize(number_bins);

  int counter = 0;

  for (auto i = node_binary_list.children("binaryDataArray").begin(); i != node_binary_list.children("binaryDataArray").end(); ++i) {

    const pugi::xml_node& bin = *i;

    MZML_BINARY_METADATA mtd = extract_mzml_binary_metadata(bin);

    pugi::xml_node node_binary = bin.child("binary");

    std::string encoded_string = node_binary.child_value();

    std::string decoded_string = decode_base64(encoded_string);

    if (mtd.compression == "gzip") {
      decoded_string = sc::decompress_zlib(decoded_string);
    }

    chrom[counter] = sc::decode_little_endian(decoded_string, mtd.precision);

    int bin_array_size = chrom[counter].size();

    if (bin_array_size != number_traces) {
      throw("Number of traces in binary array does not match the value of the chromatogram header!");
    }

    if (mtd.name == "rt") {
      pugi::xml_node node_unit = bin.find_child_by_attribute("cvParam", "unitCvRef", "UO");
      std::string unit = node_unit.attribute("unitName").as_string();

      if (unit == "minute") {
        for (double &j : chrom[counter]) {
          j *= 60;
        }
      }
    }

    counter++;
  }

  if (counter > 0) {
    for (int i = 1; i < counter; i++) {
      if (chrom[0].size() != chrom[i].size()) {
        throw("Number of traces in binary arrays of the chromatogram does not match!");
      }
    }
  }

  return chrom;
}



/*!
 * Extracts chromatograms from an mzML file according to a vector of indices,
   indicating the location of the chromatograms to retrieve.
   This is a private function of the class MZML in sc namespace.
 */
std::vector<std::vector<std::vector<double>>> sc::MZML::extract_mzml_chromatograms(const std::vector<int>& idxs) {

  std::string search_run = "//run";

  pugi::xpath_node xps_run = root.select_node(search_run.c_str());

  pugi::xml_node chrom_list = xps_run.node().child("chromatogramList");

  std::vector<std::vector<std::vector<double>>> chr;

  int number_chroms_to_extract = idxs.size();

  if (number_chroms_to_extract == 0) {
    std::cerr << "Warning: No indices given!" << std::endl;
    return chr;
  }

  if (chrom_list) {

    chr.resize(number_chroms_to_extract);

    std::vector<pugi::xml_node> chromatograms;

    for (pugi::xml_node child = chrom_list.first_child(); child; child = child.next_sibling()) {
      chromatograms.push_back(child);
    }

    for (int i = 0; i < number_chroms_to_extract; i++) {

      const int& index = idxs[i];

      const pugi::xml_node& chrom_node = chromatograms[index];

      chr[i] = extract_mzml_chromatogram(chrom_node);
    }
  }

  return chr;
}



/*!
 * Tests reading a generic XML file.
 */
void tests::test_reading_generic_xml(const std::string& file) {

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Test Reading Generic XML" << std::endl;
  std::cout << std::endl;

  sc::XML xml(file);
  std::cout << xml.file_path << std::endl;
  std::cout << xml.name << std::endl;
  std::cout << std::endl;
}



/*!
 * Tests litle endian encoding and deconding based on an input vector of doubles and a precision integer.
 */
void tests::test_encoding_decoding_little_endian(const std::vector<double>& input, const int& precision) {

    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "Encoding and Decoding Little Endian (" << precision * 8 << "-bit) Test" << std::endl;
    std::cout << std::endl;

    std::cout << "Input vector: ";
    for (double i : input) std::cout << i << " ";
    std::cout << std::endl;
    std::string result = sc::encode_little_endian(input, precision);
    std::cout << "Encoded: " << result << std::endl;
    result = sc::compress_zlib(result);
    std::cout << "Compressed: " << "Not printed!" << std::endl;
    result = sc::encode_base64(result);
    std::cout << "Encoded_base64: " << result << std::endl;

    std::cout << std::endl;
    result = sc::decode_base64(result);
    std::cout << "Decoded_base64: " << "Not printed!" << std::endl;
    result = sc::decompress_zlib(result);
    std::cout << "Decompressed: " << result << std::endl;
    std::vector<double> result_back = sc::decode_little_endian(result, precision);
    std::cout << "Decoded: ";
    for (double i : result_back) std::cout << i << " ";
    std::cout << std::endl;
    bool check = input == result_back;

    std::cout << std::endl;
    std::cout << "When 1 the result is equal to input: " << check << std::endl;
    std::cout << std::endl;
  };

/*!
 * Tests extracting spectra from mzML.
 */
void tests::test_extract_spectra_mzml(const std::string& file) {

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Test Extract Spectra mzML file:" << std::endl;
  std::cout << std::endl;

  sc::MZML mzml(file);

  std::cout << "Root name: " << mzml.get_name() << std::endl;

  std::cout << "Number of spectra: " << mzml.get_number_spectra() << std::endl;

  sc::MZML_SPECTRA_HEADERS hd;

  hd = mzml.get_spectra_headers();

  int number = hd.spec_index.size();

  std::cout << "Size of vector in headers struct: " << number << std::endl;

  std::cout << "Retention time of 10th spectrum: " << hd.scan_rt[10] << std::endl;

  std::cout << "Number of binary arrays: " << mzml.get_spectra_binary_arrays_number() << std::endl;

  std::vector<sc::MZML_BINARY_METADATA> mtd = mzml.get_spectra_binary_metadata();

  std::cout << "Name of first binary array: " << mtd[1].name << std::endl;

  std::vector<std::vector<std::vector<double>>> spectra;

  std::vector<int> indices = {10, 15};

  spectra = mzml.get_spectra(indices);

  std::cout << "Number of extracted spectra: " << spectra.size() << std::endl;

  std::cout << "Number of traces in the first extracted spectrum: " << spectra[0][0].size() << std::endl;

  std::cout << std::endl;
  };

  /*!
 * Tests extracting chromatograms from mzML.
 */
void tests::test_extract_chromatograms_mzml(const std::string& file) {

  std::cout << std::endl;
  std::cout << std::endl;
  std::cout << "Test Chromatograms mzML file:" << std::endl;
  std::cout << std::endl;

  sc::MZML mzml(file);

  std::cout << "Root name: " << mzml.get_name() << std::endl;

  std::cout << "Number of chromatograms: " << mzml.get_number_chromatograms() << std::endl;

  sc::MZML_CHROMATOGRAMS_HEADERS ch;

  ch = mzml.get_chromatograms_headers();

  int number_chroms = ch.chrom_index.size();

  std::cout << "Size of vector in headers chroms struct: " << number_chroms << std::endl;

  std::cout << "Polarity of 5th chrom: " << ch.chrom_polarity[5] << std::endl;

  std::vector<std::vector<std::vector<double>>> chroms;

  std::vector<int> indices = {1, 5, 6};

  chroms = mzml.get_chromatograms(indices);

  std::cout << "Number of extracted chroms: " << chroms.size() << std::endl;

  std::cout << "Number of variables in 1st chromatogram: " << chroms[0].size() << std::endl;

  std::cout << "Number of variables in 6th chromatogram: " << chroms[2].size() << std::endl;

  std::cout << "Number of traces in the first extracted chrom: " << chroms[0][0].size() << std::endl;

  std::cout << std::endl;

};
