//
// Created by Micael Cossa on 18/09/2025.
//

#ifndef CORE_MINESERVER_LOGIN_HANDLER_H
#define CORE_MINESERVER_LOGIN_HANDLER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <wincrypt.h>
#include <bcrypt.h>
#include <vector>
#include <iostream>
#include <cstdint>

#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "crypt32.lib")




class LoginHandler {


private:

    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    std::vector<uint8_t> der_key_bytes;

public:

    LoginHandler() {
        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, nullptr, 0) != 0)
            throw std::runtime_error("Failed to BCryptOpenAlgorithmProvider");


        if (BCryptGenerateKeyPair(hAlg, &hKey, 1024, 0) != 0)
            throw std::runtime_error("Failed to BCryptGenerateKeyPair");

        if (BCryptFinalizeKeyPair(hKey, 0) != 0)
            throw std::runtime_error("Failed to BCryptFinalizeKeyPair(");

    }

    ~LoginHandler() {

        if(!hKey)
            BCryptDestroyKey(hKey);

        if(!hAlg)
            BCryptCloseAlgorithmProvider(hAlg, 0);
    }

    bool loadServerPublicKey() {


        if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RSA_ALGORITHM, nullptr, 0) != 0) {
            std::cerr << "BCryptOpenAlgorithmProvider failed\n";
            return false;
        }

        if (BCryptGenerateKeyPair(hAlg, &hKey, 1024, 0) != 0) {
            std::cerr << "BCryptGenerateKeyPair failed\n";
            return false;
        }
        if (BCryptFinalizeKeyPair(hKey, 0) != 0) {
            std::cerr << "BCryptFinalizeKeyPair failed\n";
            return false;
        }

        // Use the legacy API wrapper: CryptExportPublicKeyInfo
        DWORD cbInfo = 0;
        if (!CryptExportPublicKeyInfo(
                (HCRYPTPROV_OR_NCRYPT_KEY_HANDLE)hKey,
                0, // dwKeySpec (not used with NCrypt handles)
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                nullptr,
                &cbInfo
        )) {
            std::cerr << "CryptExportPublicKeyInfo length query failed: " << GetLastError() << "\n";
            return false;
        }

        std::vector<BYTE> buf(cbInfo);
        auto* pkInfo = reinterpret_cast<CERT_PUBLIC_KEY_INFO*>(buf.data());


        if (!CryptExportPublicKeyInfo(
                (HCRYPTPROV_OR_NCRYPT_KEY_HANDLE)hKey,
                0,
                X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                pkInfo,
                &cbInfo
        )) {
            std::cerr << "CryptExportPublicKeyInfo failed: " << GetLastError() << "\n";
            return false;
        }

        std::cout << "Public key DER (" << cbInfo << " bytes):\n";
        for (DWORD i = 0; i < cbInfo; i++) {
            printf("%02X ", buf[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }


    }
};


#endif //CORE_MINESERVER_LOGIN_HANDLER_H
