/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef REPO_EXTENDED_ERROR_MESSAGE_HPP
#define REPO_EXTENDED_ERROR_MESSAGE_HPP

#include <boost/exception/get_error_info.hpp>
#include <sstream>

namespace repo {

template<typename E>
std::string
getExtendedErrorMessage(const E& exception)
{
  std::ostringstream errorMessage;
  errorMessage << exception.what();

  const char* const* file = boost::get_error_info<boost::throw_file>(exception);
  const int* line = boost::get_error_info<boost::throw_line>(exception);
  const char* const* func = boost::get_error_info<boost::throw_function>(exception);
  if (file && line) {
    errorMessage << " [from " << *file << ":" << *line;
    if (func) {
      errorMessage << " in " << *func;
    }
    errorMessage << "]";
  }

  return errorMessage.str();
}

} // namespace repo

#endif // REPO_EXTENDED_ERROR_MESSAGE_HPP