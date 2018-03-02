cd app
make clean
cd ..
cd gobang
make clean
cd ..
cd system
make clean
cd ..

cd app
make app.bex
cd ..
cd gobang
make gobang.bex
cd ..
cp ./app/app.bex ./system/
cp ./gobang/gobang.bex ./system/
cd system
make run

cd ..
