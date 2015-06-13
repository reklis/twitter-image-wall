# twitter image wall

proof of concept of creating wall art from media shared on twitter

the code is ugly and messy, this is mostly an art project

### building

    git submodule sync
    git submodule update --init

    cmake .
    make

### running

    export tw_consumer_key="..."
    export tw_consumer_secret="..."
    export tw_oauth_token="..."
    export tw_oauth_token_secret="..."

    # window mode
    ./twimagewall <term to track>

    # fullscreen mode
    ./twimagewall <term to track> f

