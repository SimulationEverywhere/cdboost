if test x = x$BOOST_ROOT 
then
    echo BOOST_ROOT not set
    exit 1
fi
xsltproc --nonet --xinclude bb2db.xsl simulation.xml | xsltproc --nonet db2html.xsl -
cp pre-boost.jpg ../html
cp $BOOST_ROOT/doc/src/boostbook.css ../html
cp -R $BOOST_ROOT/doc/html/images ../html
cp ../../example/main-clock.cpp ../html
