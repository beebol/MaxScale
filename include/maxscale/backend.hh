#pragma once
/*
 * Copyright (c) 2016 MariaDB Corporation Ab
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE.TXT file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2020-01-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */

#include <maxscale/cppdefs.hh>

#include <list>
#include <tr1/memory>

#include <maxscale/service.h>
#include <maxscale/session_command.hh>


namespace maxscale
{


class Backend
{
    Backend(const Backend&);
    Backend& operator =(const Backend&);
public:

    /**
     * How is the backend being closed
     */
    enum close_type
    {
        NORMAL,
        FATAL
    };

    /**
     * What type of a response we expect from the backend
     */
    enum response_type
    {
        EXPECT_RESPONSE,
        NO_RESPONSE
    };

    /**
     * @brief Create new Backend
     *
     * @param ref Server reference used by this backend
     */
    Backend(SERVER_REF* ref);

    virtual ~Backend();

    /**
     * @brief Execute the next session command in the queue
     *
     * @return True if the command was executed successfully
     */
    bool execute_session_command();

    /**
     * @brief Add a new session command to the tail of the command queue
     *
     * @param buffer   Session command to add
     * @param sequence Sequence identifier of this session command, returned when
     *                 the session command is completed
     */
    void add_session_command(GWBUF* buffer, uint64_t sequence);

    /**
     * @brief Mark the current session command as successfully executed
     *
     * This should be called when the response to the command is received
     *
     * @return The sequence identifier for this session command
     */
    uint64_t complete_session_command();

    /**
     * @brief Check if backend has session commands
     *
     * @return True if backend has session commands
     */
    size_t session_command_count() const;

    /**
     * @brief Get pointer to server reference
     *
     * @return Pointer to server reference
     */
    SERVER_REF* backend() const;

    /**
     * @brief Create a new connection
     *
     * @param session The session to which the connection is linked
     *
     * @return True if connection was successfully created
     */
    bool connect(MXS_SESSION* session);

    /**
     * @brief Close the backend
     *
     * This will close all active connections created by the backend.
     */
    void close(close_type type = NORMAL);

    /**
     * @brief Get a pointer to the internal DCB
     *
     * @return Pointer to internal DCB
     */
    DCB* dcb() const;

    /**
     * @brief Write data to the backend server
     *
     * @param buffer          Buffer containing the data to write
     * @param expect_response Whether to expect a response to the query
     *
     * @return True if data was written successfully
     */
    bool write(GWBUF* buffer, response_type type = EXPECT_RESPONSE);

    /**
     * @brief Write an authentication switch request to the backend server
     *
     * @param buffer Buffer containing the authentication switch request
     *
     * @return True if request was successfully written
     */
    bool auth(GWBUF* buffer);

    /**
     * @brief Mark that a reply to a query was received and processed
     */
    void ack_write();

    /**
     * @brief Store a command
     *
     * The command is stored and executed once the session can execute
     * the next command.
     *
     * @param buffer Buffer to store
     */
    void store_command(GWBUF* buffer);

    /**
     * @brief Write the stored command to the backend server
     *
     * @return True if command was written successfully
     */
    bool write_stored_command();

    /**
     * @brief Check if backend is in use
     *
     * @return True if backend is in use
     */
    bool in_use() const;

    /**
     * @brief Check if backend is waiting for a result
     *
     * @return True if backend is waiting for a result
     */
    bool is_waiting_result() const;

    /**
     * @brief Check if the backend is closed
     *
     * @return True if the backend is closed
     */
    bool is_closed() const;

private:
    /**
     * Internal state of the backend
     */
    enum backend_state
    {
        IN_USE           = 0x01, /**< Backend has been taken into use */
        WAITING_RESULT   = 0x02, /**< Waiting for a reply */
        CLOSED           = 0x04, /**< Backend is no longer in use */
        FATAL_FAILURE    = 0x08  /**< Backend references that should be dropped */
    };

    /**
     * @brief Clear state
     *
     * @param state State to clear
     */
    void clear_state(backend_state state);

    /**
     * @brief Set state
     *
     * @param state State to set
     */
    void set_state(backend_state state);


    bool               m_closed;           /**< True if a connection has been opened and closed */
    SERVER_REF*        m_backend;          /**< Backend server */
    DCB*               m_dcb;              /**< Backend DCB */
    mxs::Buffer        m_pending_cmd;      /**< Pending commands */
    int                m_state;            /**< State of the backend */
    SessionCommandList m_session_commands; /**< List of session commands that are
                                            * to be executed on this backend server */
};

typedef std::tr1::shared_ptr<Backend> SBackend;
typedef std::list<SBackend> BackendList;
}