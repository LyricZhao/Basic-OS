cd app
make clean
make app.bex

cd ..
cp ./app/app.bex ./system/

cd system
make clean
make img
make run

cd ..
