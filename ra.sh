cd app
make clean
cd ..
cd system
make clean
cd ..

cd app
make app.bex
cd ..
cp ./app/app.bex ./system/
cd system
make run

cd ..
