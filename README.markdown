# twitter image wall

proof of concept of creating wall art from media shared on twitter

the code is ugly and messy, this is mostly an art project

### building

    git submodule sync
    git submodule update --init

    cmake .
    make

### running

    export .env # with twitter keys
    ./twimagewall <term to track> [fullscreen]

