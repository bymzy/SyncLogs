//
// Created by root on 1/21/18.
//

#ifndef PROJECT_ERR_HPP
#define PROJECT_ERR_HPP

class Err
{
public:
    enum _err {
        /* database error */
        E_TABLE_NOT_EXIST = 1000,
        E_TABLE_EXIST,
        E_KEY_NOT_EXIST,
        E_KEY_EXIST,

        /* logic error */
    };
};

#endif //PROJECT_ERR_HPP
