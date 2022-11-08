#include <filesystem>
#include <utility>

#include <plateau/udx/udx_file_collection.h>
#include <fstream>
#include <queue>
#include <set>
#include <regex>
#include "plateau/geometry/geo_reference.h"

namespace plateau::udx {
    namespace fs = std::filesystem;

    PredefinedCityModelPackage UdxSubFolder::getPackage(const std::string& folder_name) {
        if (folder_name == bldg) return PredefinedCityModelPackage::Building;
        if (folder_name == tran) return PredefinedCityModelPackage::Road;
        if (folder_name == urf) return PredefinedCityModelPackage::UrbanPlanningDecision;
        if (folder_name == luse) return PredefinedCityModelPackage::LandUse;
        if (folder_name == frn) return PredefinedCityModelPackage::CityFurniture;
        if (folder_name == veg) return PredefinedCityModelPackage::Vegetation;
        if (folder_name == dem) return PredefinedCityModelPackage::Relief;
        if (folder_name == fld) return PredefinedCityModelPackage::DisasterRisk;
        if (folder_name == tnm) return PredefinedCityModelPackage::DisasterRisk;
        if (folder_name == lsld) return PredefinedCityModelPackage::DisasterRisk;
        if (folder_name == htd) return PredefinedCityModelPackage::DisasterRisk;
        if (folder_name == ifld) return PredefinedCityModelPackage::DisasterRisk;
        return PredefinedCityModelPackage::Unknown;
    }

    CityModelPackageInfo UdxSubFolder::getPackageInfo(const std::string& folder_name) {
        return CityModelPackageInfo::getPredefined(getPackage(folder_name));
    }

    std::shared_ptr<UdxFileCollection> UdxFileCollection::find(const std::string& source) {
        auto result = std::make_shared<UdxFileCollection>();
        find(source, *result);
        return result;
    }

    namespace {
        /**
         * 指定パスを起点に、幅優先探索(BFS)でGMLファイルを検索します。
         * 検索の高速化のため、GMLファイルの配置場所の深さはすべて同じであるという前提に立ち、
         * 最初のGMLファイルが見つかった地点でこれ以上深いフォルダの探索は中止します。
         * 同じ深さにある別のフォルダは探索対象とします。
         * @param dir_path  検索の起点となるパスです。
         * @param result 結果はこの vector に追加されます。
         * @return GMLファイルのパスの vector です。
         */
        void findGMLsBFS(const std::string& dir_path, std::vector<GmlFileInfo>& result) {
            auto queue = std::queue<std::string>();
            queue.push(dir_path);
            bool push_more_dir = true;
            while (!queue.empty()) {
                auto next_dir = queue.front();
                queue.pop();
                // ファイルから検索します
                for (const auto& entry : fs::directory_iterator(next_dir)) {
                    if (entry.is_directory()) continue;
                    const auto& path = entry.path();
                    if (path.extension() == ".gml") {
                        result.emplace_back(path.string());
                        // 最初のGMLファイルが見つかったら、これ以上探索キューに入れないようにします。
                        // 同じ深さにあるフォルダはすでにキューに入っているので、「深さは同じだけどフォルダが違う」という状況は検索対象に含まれます。
                        push_more_dir = false;
                    }
                }
                // 次に探索すべきディレクトリをキューに入れます。
                if (!push_more_dir) continue;
                for (const auto& entry : fs::directory_iterator(next_dir)) {
                    if (!entry.is_directory()) continue;
                    queue.push(entry.path().string());
                }
            }
        }
    }

    void UdxFileCollection::find(const std::string& source, UdxFileCollection& collection) {
        collection.udx_path_ = fs::u8path(source).append(u"udx").make_preferred().u8string();
        // udxフォルダ内の各フォルダについて
        for (const auto& entry : fs::directory_iterator(collection.udx_path_)) {
            if (!entry.is_directory()) continue;
            auto udx_sub_folder = UdxSubFolder(entry.path().filename().string());
            const auto package = udx_sub_folder.getPackage(udx_sub_folder.getName());
            auto& file_map = collection.files_;
            if (file_map.count(package) == 0) {
                file_map.emplace(package, std::vector<GmlFileInfo>());
            }
            auto& gml_files = collection.files_.at(package);
            findGMLsBFS(entry.path().string(), gml_files);
        }
    }

    std::shared_ptr<UdxFileCollection> UdxFileCollection::filter(const geometry::Extent& extent) const {
        auto result = std::make_shared<UdxFileCollection>();
        filter(extent, *result);
        return result;
    }

    void UdxFileCollection::filter(const geometry::Extent& extent, UdxFileCollection& collection) const {
        std::vector<MeshCode> mesh_codes;
        MeshCode::getThirdMeshes(extent, mesh_codes);
        std::set<std::string> second_mesh_strings;

        // 2次メッシュのリスト取得
        for (const auto& mesh_code : mesh_codes) {
            second_mesh_strings.insert(mesh_code.asSecond().get());
        }

        for (const auto& second_mesh_string : second_mesh_strings) {
            mesh_codes.emplace_back(second_mesh_string);
        }

        filterByMeshCodes(mesh_codes, collection);
    }

    void UdxFileCollection::filterByMeshCodes(const std::vector<MeshCode>& mesh_codes,
                                              UdxFileCollection& collection) const {
        // これがないとフィルターの結果に対して fetch を実行するときにパスがずれます。
        collection.setUdxPath(udx_path_);
        // 検索用に、引数の mesh_codes を文字列のセットにします。
        auto mesh_codes_str_set = std::set<std::string>();
        for (const auto& mesh_code : mesh_codes) {
            mesh_codes_str_set.insert(mesh_code.get());
        }
        // ファイルごとに mesh_codes_str_set に含まれるなら追加していきます。
        for (const auto& [sub_folder, files] : files_) {
            for (const auto& file : files) {
                if (mesh_codes_str_set.find(file.getMeshCode().get()) != mesh_codes_str_set.end()) {
                    collection.addFile(sub_folder, file);
                }
            }
        }
    }

    std::shared_ptr<UdxFileCollection>
        UdxFileCollection::filterByMeshCodes(const std::vector<MeshCode>& mesh_codes) const {
        auto result = std::make_shared<UdxFileCollection>();
        filterByMeshCodes(mesh_codes, *result);
        return result;
    }

    PredefinedCityModelPackage UdxFileCollection::getPackages() {
        auto result = PredefinedCityModelPackage::None;
        for (const auto& [key, _] : files_) {
            result = result | key;
        }
        return result;
    }

    const std::string& UdxFileCollection::getGmlFilePath(PredefinedCityModelPackage package, int index) {
        return getGmlFileInfo(package, index).getPath();
    }

    const GmlFileInfo& UdxFileCollection::getGmlFileInfo(PredefinedCityModelPackage package, int index) {
        if (files_.find(package) == files_.end())
            throw std::out_of_range("Key not found");

        if (index >= files_[package].size())
            throw std::out_of_range("Index out of range");

        return files_[package][index];
    }

    int UdxFileCollection::getGmlFileCount(PredefinedCityModelPackage package) {
        if (files_.find(package) == files_.end())
            throw std::out_of_range("Key not found");

        return (int)files_[package].size();
    }

    std::shared_ptr<std::vector<std::string>> UdxFileCollection::getGmlFiles(PredefinedCityModelPackage package) {
        const auto result = std::make_shared<std::vector<std::string>>();

        if (files_.find(package) == files_.end())
            return result;

        for (const auto file : files_[package]) {
            result->push_back(file.getPath());
        }
        return result;
    }

    // fetch で使う無名関数
    namespace {
        using ConstStrIterT = decltype(std::string("a").cbegin());

        /**
        * @brief 正規表現で検索しますが、ヒントを与えることで検索を高速化します。
        *        ヒントとは何かについては、 関数 searchAllStringsBetween のコメントを参照してください。
        * @param str 検索対象の文字列です。
        * @param search_pos 検索の開始位置です。
        * @param matched 検索結果はここに格納されます。
        * @param regex 検索する正規表現です。
        * @param hint ヒント文字列です。正規表現が検索ヒットするとき、ヒント文字列が必ず検索ヒットし、
        *             かつそのヒント文字列の周囲の指定バイト数にその正規表現ヒット文字列が含まれることが条件となります。
        * @param search_range_before_hint ヒント文字列の前の何バイト目から正規表現による検索を始めるかです。
        * @param search_range_after_hint ヒント文字列の後の何バイト目まで正規表現による検索対象にするかです。
        * @return ヒットしたらtrue, なければ false を返します。
        */
        bool regexSearchWithHint(const std::string& str, ConstStrIterT search_pos, std::smatch& matched,
                                 const std::regex& regex, const std::string& hint,
                                 unsigned search_range_before_hint, unsigned search_range_after_hint
        ) {
            const auto str_begin = str.cbegin();
            while (search_pos != str.cend()) {
                // ヒントで検索します。
                auto hint_matched_pos = str.find(hint, search_pos - str_begin);
                // ヒントで検索ヒットしなければ、正規表現でも検索ヒットしません。そのようなヒントが渡されていることが前提です。
                if (hint_matched_pos == std::string::npos) return false;
                // ヒントが検索ヒットしたので、その周囲の指定数のバイト範囲を正規表現の検索範囲にします。
                auto search_start =
                    str_begin + std::max((long long)0, (long long)hint_matched_pos - search_range_before_hint);
                auto search_end = std::min(str.end(), str_begin + (long long)hint_matched_pos + (long long)hint.size() + search_range_after_hint);
                // 正規表現でヒットしたら、その結果を引数 matched に格納して返します。
                bool found = std::regex_search(search_start, search_end, matched, regex);
                if (found) return true;
                // ヒントにはヒットしたけど正規表現にヒットしなかったケースです。検索位置を進めて再度ヒントを検索します。
                search_pos = std::min(str.cend(), str_begin + (long long)hint_matched_pos + (long long)hint.size());
            }

        }

        /**
        * 引数文字列 str のうち、引数 begin_tag_regex と end_tag_regex で囲まれた文字列をすべて検索し set で返します。
        * end_tag_regex は begin_tag_regex が登場する箇所より後が検索対象となります。
        * begin_tag_regex に対応する end_tag_regex がない場合、strの末尾までが対象となります。
        * 検索結果のうち同じ文字列は1つにまとめられます。
        *
        * ヒントについて:
        * 検索の高速化のために引数でヒント文字列を与える必要があります。
        * 例えば40MBのGMLファイルに対して愚直に正規表現で検索すると1分30秒程度の時間がかかります。
        * 引数でヒントを与えることで、正規表現の検索範囲が狭まり 4秒程度に短縮できました。
        * ヒント文字列とは、正規表現が検索ヒットするとき、その場所でヒント文字列も必ずヒットする、という条件を満たす文字列です。
        * 例えば、文字列 <start_tag> を検索したいが、<括弧> の前後に半角スペースが入っているケースも検索したいという場合、
        * 検索正規表現は < *start_tag *> であり、ヒント文字列は start_tag となります。
        * このとき、まず正規表現検索よりも高速な通常文字列検索で start_tag が検索されます。
        * そして見つかった位置から前後に指定バイト数の幅をとった範囲を正規表現で検索します。
        * その範囲は引数 search_range_before_hint, search_range_after_hint で指定します。この値は少ないほうが速くなります。
        */
        std::set<std::string> searchAllStringsBetween(
                const std::regex& begin_tag_regex, const std::regex& end_tag_regex,
                const std::string& str,
                const std::string& begin_tag_hint, const std::string& end_tag_hint,
                unsigned search_range_before_hint, unsigned search_range_after_hint
        ) {
            std::set<std::string> found;
            std::smatch begin_tag_matched;
            std::smatch end_tag_matched;
            auto begin_tag_search_iter = str.begin();
            while (true) {
                // 開始タグを検索します。
                if (!regexSearchWithHint(str, begin_tag_search_iter, begin_tag_matched, begin_tag_regex, begin_tag_hint,
                    search_range_before_hint, search_range_after_hint)) {
                    break;
                }
                // 終了タグを検索します。
                const auto next_of_begin_tag = begin_tag_matched[0].second;
                if (regexSearchWithHint(str, next_of_begin_tag, end_tag_matched, end_tag_regex, end_tag_hint,
                    search_range_before_hint, search_range_after_hint)) {
                    // 開始タグと終了タグに挟まれた文字列を結果として格納します。
                    found.insert(std::string(next_of_begin_tag, end_tag_matched[0].first));
                } else {
                    found.insert(std::string(next_of_begin_tag, str.end()));
                    break;
                }
                const auto next_of_end_tag = end_tag_matched[0].second;
                // イテレーターを進めます。
                begin_tag_search_iter = next_of_end_tag;
            }
            return found;
        }

        std::string loadFile(const fs::path& file_path) {
            std::ifstream ifs(file_path.u8string());
            if (!ifs) {
                throw std::runtime_error(
                        "loadFile : Could not open file " + file_path.u8string());
            }
            std::ostringstream buffer;
            buffer << ifs.rdbuf();
            return buffer.str();
        }

        auto regex_options = std::regex::optimize | std::regex::nosubs;

        std::set<std::string> searchAllImagePathsInGML(const std::string& file_content) {
            // 開始タグは <app:imageURI> です。ただし、<括弧> の前後に半角スペースがあっても良いものとします。
            static const auto begin_tag = std::regex(R"(< *app:imageURI *>)", regex_options);
            // 終了タグは </app:imageURI> です。ただし、<括弧> と /(スラッシュ) の前後に半角スペースがあっても良いものとします。
            static const auto end_tag = std::regex(R"(< */ *app:imageURI *>)", regex_options);
            static auto tag_hint = std::string("app:imageURI");
            auto found_url_strings = searchAllStringsBetween(begin_tag, end_tag, file_content, tag_hint, tag_hint, 5, 10);
            return found_url_strings;
        }

        std::set<std::string> searchAllCodelistPathsInGML(const std::string& file_content) {
            // 開始タグは codeSpace=" です。ただし =(イコール), "(ダブルクォーテーション)の前後に半角スペースがあっても良いものとします。
            static const auto begin_tag = std::regex(R"(codeSpace *= *")", regex_options);
            // 終了タグは、開始タグの次の "(ダブルクォーテーション)です。
            static const auto end_tag = std::regex(R"(")", regex_options);
            static const auto begin_tag_hint = "codeSpace";
            auto found_strings = searchAllStringsBetween(begin_tag, end_tag, file_content, begin_tag_hint, "\"", 5, 10);
            return found_strings;
        }

        /**
         * 引数の set の中身を相対パスと解釈し、 setの各要素をコピーします。
         * 相対パスの基準は コピー元は 引数 src_base_path、 コピー先は dest_base_path になります。
         * コピー先に同名のフォルダが存在する場合はコピーしません。
         * コピー元が実在しない場合はコピーしません。
         */
        void copyFiles(const std::set<std::string>& path_set, const fs::path& src_base_path, const fs::path& dest_base_path) {
            for (const auto& path : path_set) {
                auto src = src_base_path;
                auto dest = dest_base_path;
                src.append(path).make_preferred();
                dest.append(path).make_preferred();
                if (!fs::exists(src)) {
                    std::cout << "file not exist : " << src.string() << std::endl;
                    continue;
                }
                fs::create_directories(dest.parent_path());
                fs::copy(src, dest, fs::copy_options::skip_existing);
            }
        }
    } // fetch で使う無名関数

    std::shared_ptr<GmlFileInfo> UdxFileCollection::fetch(const std::string& destination_root_path,
        const GmlFileInfo& gml_file) {
        auto result = std::make_shared<GmlFileInfo>("");
        fetch(destination_root_path, gml_file, *result);
        return result;
    }

    void UdxFileCollection::fetch(
        const std::string& destination_root_path, const GmlFileInfo& gml_file,
        GmlFileInfo& copied_gml_file) {

        const auto udx_path_len = gml_file.getPath().rfind("udx") + 3;
        if (udx_path_len == std::string::npos) {
            throw std::runtime_error("Invalid gml path. Could not find udx folder");
        }

        const auto udx_path = gml_file.getPath().substr(0, udx_path_len);
        const auto gml_relative_path = fs::relative(fs::u8path(gml_file.getPath()).make_preferred(), fs::u8path(udx_path)).make_preferred().string();

        const auto root_folder_name = fs::u8path(udx_path).parent_path().filename().string();
        auto destination_root = fs::u8path(destination_root_path);
        const auto destination_udx_path = destination_root.append(root_folder_name).append("udx").string();
        fs::path gml_destination_path(destination_udx_path);
        gml_destination_path.append(gml_relative_path);
        fs::create_directories(gml_destination_path.parent_path());
        const auto& gml_file_path = gml_file.getPath();
        try {
            fs::copy(gml_file_path, gml_destination_path, fs::copy_options::skip_existing);
        }
        catch (...) {
        }

        // GMLファイルを読み込み、関連するテクスチャパスとコードリストパスを取得します。
        const auto gml_content = loadFile(gml_file.getPath());
        auto image_paths = searchAllImagePathsInGML(gml_content);
        auto codelist_paths = searchAllCodelistPathsInGML(gml_content);

        for (const auto& path : image_paths) {
            std::cout << path << std::endl;
        }
        for (const auto& path : codelist_paths) {
            std::cout << path << std::endl;
        }

        // テクスチャとコードリストファイルをコピーします。
        auto gml_dir_path = fs::path(gml_file_path).parent_path();
        auto relative_gml_dir_path = fs::path(gml_relative_path).parent_path().u8string();
        auto app_destination_path = fs::path(destination_udx_path).append(relative_gml_dir_path);
        copyFiles(image_paths, gml_dir_path, app_destination_path);
        copyFiles(codelist_paths, gml_dir_path, app_destination_path);

        copied_gml_file.setPath(gml_destination_path.u8string());
    }

    std::string UdxFileCollection::getU8RelativePath(const std::string& path) const {
        return fs::relative(fs::u8path(path), fs::u8path(udx_path_)).u8string();
    }

    TVec3d UdxFileCollection::calculateCenterPoint(const geometry::GeoReference& geo_reference) {
        const auto& mesh_codes = getMeshCodes();
        double lat_sum = 0;
        double lon_sum = 0;
        double height_sum = 0;
        for (const auto& mesh_code : mesh_codes) {
            const auto& center = mesh_code.getExtent().centerPoint();
            lat_sum += center.latitude;
            lon_sum += center.longitude;
            height_sum += center.height;
        }
        auto num = (double)mesh_codes.size();
        geometry::GeoCoordinate geo_average = geometry::GeoCoordinate(lat_sum / num, lon_sum / num, height_sum / num);
        auto euclid_average = geo_reference.project(geo_average);
        return euclid_average;
    }

    std::string UdxFileCollection::getRelativePath(const std::string& path) const {
        return fs::relative(fs::u8path(path).make_preferred(), fs::u8path(udx_path_)).make_preferred().string();
    }

    std::set<MeshCode>& UdxFileCollection::getMeshCodes() {
        if (!mesh_codes_.empty())
            return mesh_codes_;

        for (const auto& [_, files] : files_) {
            for (const auto& file : files) {
                mesh_codes_.insert(file.getMeshCode());
            }
        }
        return mesh_codes_;
    }

    void UdxFileCollection::addFile(PredefinedCityModelPackage sub_folder, const GmlFileInfo& gml_file_info) {
        if (files_.count(sub_folder) <= 0) {
            files_.emplace(sub_folder, std::vector<GmlFileInfo>());
        }
        files_.at(sub_folder).push_back(gml_file_info);
    }

    void UdxFileCollection::setUdxPath(std::string udx_path) {
        udx_path_ = std::move(udx_path);
    }
}