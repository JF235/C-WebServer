#! /bin/bash

criar_html() {
    cat > $1 <<- EOM
<!DOCTYPE html>
<html>
    <head>
        <title>$1</title>
    </head>
    <body>
        <h1>Conteudo do $1</h1>
    </body>
</html>
EOM
}

#===========================================================================
#===========================================================================
#===========================================================================
#===========================================================================

webspacepath="$PWD/testando"
passwdpath="$PWD/web/.passwd"
passwdpath1="$PWD/web/senhas1/.passwd"
rm -rf $webspacepath
mkdir $webspacepath

# Pasta: meu-webspace
cd $webspacepath
criar_html index.html
mkdir dir-aberto1
mkdir dir-protegido1

# Pasta: meu-webspace/dir-aberto1
cd "$webspacepath/dir-aberto1"
criar_html aberto1.html
criar_html fechado1.html
chmod ugo-rw fechado1.html
mkdir dir-protegido2

# Pasta: meu-webspace/dir-aberto1/dir-protegido2
cd dir-protegido2
touch .htaccess
echo $passwdpath > .htaccess
criar_html welcome.html
criar_html fechado2.html
chmod ugo-rw fechado2.html

# Pasta meu-webspace/dir-protegido1
cd "$webspacepath/dir-protegido1"
touch .htaccess
echo $passwdpath > .htaccess
criar_html protegido1.html
mkdir dir-semvarr # sem varredura

# Pasta meu-webspace/dir-protegido1/dir-semvarr
cd dir-semvarr
criar_html "index.html"
mkdir dir-protegido3

# Pasta meu-webspace/dir-protegido1/dir-semvarr/dir-protegido3
cd dir-protegido3
touch .htaccess
echo $passwdpath1 > .htaccess
criar_html "superprotegido.html"

chmod ugo-x "$webspacepath/dir-protegido1/dir-semvarr"
