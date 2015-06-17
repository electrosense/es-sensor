openssl pkcs12 -export -out keystore.p12 -inkey Collector-SSL-SK.pem -in Collector-SSL-Cert.pem
keytool -importkeystore -destkeystore keystore.jks -srcstoretype PKCS12 -srckeystore keystore.p12
keytool -importcert -trustcacerts -file CA-Cert.pem -keystore truststore.jks
