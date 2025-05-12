#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <vector>

namespace fs = std::filesystem;

#ifdef _WIN32
const std::string SO = "Windows";
const char PATH_SEP = '\\'; // Separador de caminho no Windows
#elif __APPLE__ || __linux__
const std::string SO = "Unix";
const char PATH_SEP = '/'; // Separador no macOS/Linux
#else
const std::string SO = "Desconhecido";
#endif

// Converter string para wstring corretamente
std::wstring stringParaWide(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

// Converter wstring para string corretamente
std::string wideParaString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

// Remover espaços extras no nome
std::wstring removerEspacos(std::wstring nome) {
    nome.erase(std::remove(nome.begin(), nome.end(), L' '), nome.end()); // Remove espaços em branco
    return nome;
}

// Substituir caracteres proibidos e garantir compatibilidade entre SOs
std::wstring substituirCaracteres(const std::wstring& nome) {
    std::unordered_map<wchar_t, wchar_t> substituicoes = {{L'A', L'a'}, {L'Ç', L'c'}, {L'ç', L'c'}, {L'1', L'0'}, {L'G', L'g'}, {L'ã', L'a'}, {L'õ', L'o'}};
    std::wstring proibidosWin = L"<>|:*?\"";
    std::wstring proibidosUnix = L"/";

    std::wstring novoNome = nome;
    
    for (wchar_t& c : novoNome) {
        if (substituicoes.count(c)) c = substituicoes[c];
        if ((SO == "Windows" && proibidosWin.find(c) != std::wstring::npos) || 
            (SO == "Unix" && proibidosUnix.find(c) != std::wstring::npos)) {
            c = L'_'; // Substitui caracteres proibidos por "_"
        }
    }
    return removerEspacos(novoNome);
}

// Função para renomear arquivos e pastas percorrendo todas as subpastas
void renomearArquivos(const fs::path& caminhoBase) {
    try {
        // **Lista para armazenar arquivos e diretórios separadamente**
        std::vector<fs::path> arquivos;
        std::vector<fs::path> diretorios;

        // **Primeiro, coleta todos os caminhos**
        for (const auto& entrada : fs::recursive_directory_iterator(caminhoBase, fs::directory_options::skip_permission_denied)) {
            if (entrada.is_directory()) {
                diretorios.push_back(entrada.path()); // Adiciona diretórios à lista
            } else {
                arquivos.push_back(entrada.path());  // Adiciona arquivos à lista
            }
        }

        // **Agora, renomeia arquivos primeiro**
        for (const auto& caminhoOriginal : arquivos) {
            std::wstring nomeSemExtensao = stringParaWide(caminhoOriginal.stem().string());
            std::wstring extensao = stringParaWide(caminhoOriginal.extension().string());

            std::wstring novoNomeSemExtensao = substituirCaracteres(nomeSemExtensao);
            fs::path novoCaminho = caminhoOriginal.parent_path() / fs::path(wideParaString(novoNomeSemExtensao + extensao));

            if (caminhoOriginal != novoCaminho) {
                try {
                    fs::rename(caminhoOriginal, novoCaminho);
                    std::wcout << L"Renomeado: " << caminhoOriginal << L" -> " << novoCaminho << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao renomear " << caminhoOriginal << ": " << e.what() << std::endl;
                }
            }
        }

        // **Depois, renomeia diretórios em ordem reversa (dos mais profundos para os pais)**
        std::reverse(diretorios.begin(), diretorios.end());  // Evita perder referências das subpastas

        for (const auto& caminhoOriginal : diretorios) {
            std::wstring nomeSemExtensao = stringParaWide(caminhoOriginal.filename().string());
            std::wstring novoNomeSemExtensao = substituirCaracteres(nomeSemExtensao);
            fs::path novoCaminho = caminhoOriginal.parent_path() / fs::path(wideParaString(novoNomeSemExtensao));

            if (caminhoOriginal != novoCaminho) {
                try {
                    fs::rename(caminhoOriginal, novoCaminho);
                    std::wcout << L"Renomeado: " << caminhoOriginal << L" -> " << novoCaminho << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Erro ao renomear " << caminhoOriginal << ": " << e.what() << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erro ao acessar diretório: " << e.what() << std::endl;
    }
}

int main() {
    setlocale(LC_ALL, "pt_BR.UTF-8");
    std::string diretorio = (SO == "Windows") ? "D:\\Temp_Rename\\Aline" : "/Users/aline";
    std::cout << "Sistema operacional detectado: " << SO << std::endl;

    if (fs::exists(diretorio)) {
        renomearArquivos(diretorio);
    } else {
        std::cerr << "Diretório não encontrado!" << std::endl;
    }

    return 0;
}