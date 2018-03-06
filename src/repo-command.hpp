/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2014-2018, Regents of the University of California.
 *
 * This file is part of NDN repo-ng (Next generation of NDN repository).
 * See AUTHORS.md for complete list of repo-ng authors and contributors.
 *
 * repo-ng is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * repo-ng is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * repo-ng, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef REPO_REPO_COMMAND_HPP
#define REPO_REPO_COMMAND_HPP

#include "common.hpp"
#include "repo-command-parameter.hpp"

#include <stdexcept>

namespace repo {

class RepoCommand : boost::noncopyable
{
public:
  /** \brief represents an error in RepoCommandParameters
   */
  class ArgumentError : public std::invalid_argument
  {
  public:
    explicit
    ArgumentError(const std::string& what)
      : std::invalid_argument(what)
    {
    }
  };

  virtual
  ~RepoCommand() = default;

  class FieldValidator
  {
  public:
    FieldValidator();

    /** \brief declare a required field
     */
    FieldValidator&
    required(RepoParameterField field)
    {
      m_required[field] = true;
      return *this;
    }

    /** \brief declare an optional field
     */
    FieldValidator&
    optional(RepoParameterField field)
    {
      m_optional[field] = true;
      return *this;
    }

    /** \brief verify that all required fields are present,
     *         and all present fields are either required or optional
     *  \throw ArgumentError
     */
    void
    validate(const RepoCommandParameter& parameters) const;

  private:
    std::vector<bool> m_required;
    std::vector<bool> m_optional;
  };

  void
  validateRequest(const RepoCommandParameter& parameters);

private:
  virtual void
  check(const RepoCommandParameter& parameters) const
  {
  }

public:
  FieldValidator m_requestValidator;
};

class InsertCommand : public RepoCommand
{
public:
  InsertCommand();
};

class InsertCheckCommand : public RepoCommand
{
public:
  InsertCheckCommand();
};

class DeleteCommand : public RepoCommand
{
public:
  DeleteCommand();

private:
  void
  check(const RepoCommandParameter& parameters) const override;
};

} // namespace repo

#endif // REPO_REPO_COMMAND_HPP