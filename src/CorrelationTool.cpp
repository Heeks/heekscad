// CorrelationTool.cpp
/*
 * Copyright (c) 2009, Dan Heeks, Perttu Ahola
 * This program is released under the BSD license. See the file COPYING for
 * details.
 */
#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <string>
#include <set>
#include <map>
#include <list>

#include "../interface/Box.h"
#include "../interface/HeeksObj.h"
#include "../interface/HeeksCADInterface.h"
#include "CorrelationTool.h"

#include <sstream>

extern CHeeksCADInterface heekscad_interface;

/**
	This routine determines how different the two bounding boxes are in size.  If the difference
	is less than the max_scale_threshold (either bigger or smaller) then it returns 'true'
	indicating that the two objects are of a similar size.

	It also returns the scale factor required to be applied to the sample bounding box to make
	it the same size as the reference object.
 */
bool CCorrelationTool::SimilarScale( 
	const CBox &reference_box, 
	const CBox &sample_box, 
	const double max_scale_threshold,
	double *pRequiredScaling ) const
{
	// Calculate the diagonal distance for each bounding box.  This is as good an estimate
	// as any.

	double sample_diagonal = sqrt( 	(sample_box.Width() * sample_box.Width()) +
					(sample_box.Height() * sample_box.Height()));

	double reference_diagonal = sqrt( 	(reference_box.Width() * reference_box.Width()) +
						(reference_box.Height() * reference_box.Height()));

	if (sample_diagonal > reference_diagonal)
	{
		// It's bigger. How much do we need to scale it down?
		double required_scaling = reference_diagonal / sample_diagonal;

		if (pRequiredScaling) *pRequiredScaling = required_scaling;
		return( (1.0 / required_scaling) < max_scale_threshold);
	} // End if - then
	else
	{
		// It's smaller. How much do we need to scale it up?
		double required_scaling = reference_diagonal / sample_diagonal;

		if (pRequiredScaling) *pRequiredScaling = required_scaling;
		return( required_scaling < max_scale_threshold);
	} // End if - else
} // End SimilarScale() method


/**
	Get a set of correlation points that represent a single object.
 */
CCorrelationTool::CorrelationData_t CCorrelationTool::CorrelationData(      const CCorrelationTool::Symbol_t & sample_symbol,
                                                const CCorrelationTool::Symbol_t & reference_symbol,
                                                const int number_of_sample_points,
                                                const double max_scale_threshold ) const
{
	CorrelationData_t results;
	double required_scaling = 1.0;	// How much do we need to scale the sample object to make it the same
					// size as the reference object?  We need this value so that we can
					// scale the distance values as well.

	HeeksObj *sample_object = heekscad_interface.GetIDObject( sample_symbol.first, sample_symbol.second );
	if (! sample_object)
	{
		// Couldn't find sample.  Return an empty set.
		// printf("Couldn't find sample object for type='%d', id='%d'\n", sample_symbol.first, sample_symbol.second );
		return(results);
	} // End if - then

	HeeksObj *reference_object = heekscad_interface.GetIDObject( reference_symbol.first, reference_symbol.second );
	if (! reference_object)
	{
		// Couldn't find reference.  Return an empty set.
		// printf("Couldn't find reference object for type='%d', id='%d'\n", reference_symbol.first, reference_symbol.second );
		return(results);
	} // End if - then

	// Get each symbol's bounding box.
	CBox reference_box, sample_box;
	reference_object->GetBox(reference_box);
	sample_object->GetBox(sample_box);

	// First see if they're close enough in size to be a possible match.
	if (! SimilarScale( reference_box, sample_box, max_scale_threshold, &required_scaling ))
	{
		// They're too different in size.
		// printf("Rejecting on size basis. Scale required is %lf\n", required_scaling );
		return(results);	// return an empty data set.
	} // End if - then

	// printf("Required scaling is %lf\n", required_scaling);

	// They're close enough in scale to warrant further investigation.  Now roll up your sleeves and
	// describe this thar critter.

	for (int sample=0; sample<number_of_sample_points; sample++)
	{
		// We want to draw a line from the centre of the bounding box out at this
		// sample's angle for a length that is long enough to overlap the bounding box's edge.
		double angle = (double(sample) / 360.0);
	
		double radius = ((max_scale_threshold * reference_box.Width()) + (max_scale_threshold * reference_box.Height()));
		double alpha = 3.1415926 * 2 / number_of_sample_points;
                double theta = alpha * sample;
		double sample_centroid[3];
		sample_box.Centre( sample_centroid );

		// Construct a line from the centre of the sample object to somewhere away from it at this sample's angle.  Make
		// sure we're comparing both sets on the same Z plane.  We will worry about rotation matrices when I get
		// smarter.

		// sample_centroid[2] = 0.0;
                double verification_line_end[3] = { (cos( theta ) * radius) + sample_centroid[0], (sin( theta ) * radius) + sample_centroid[1], sample_centroid[2] };

		// Now find all intersection points between this verification line and the sample object.
		HeeksObj* verification_line = heekscad_interface.NewLine(sample_centroid, verification_line_end);
		if (verification_line)
		{
			std::list<double> intersections;
                        if (sample_object->Intersects( verification_line, &intersections ))
                        {
				// printf("Found %d intersections\n", intersections.size() / 3);

				CorrelationSample_t correlation_sample;
                                while (((intersections.size() % 3) == 0) && (intersections.size() > 0))
                                {
                                        Point3d intersection;

                                        intersection.x = *(intersections.begin());
                                        intersections.erase(intersections.begin());

                                        intersection.y = *(intersections.begin());
                                        intersections.erase(intersections.begin());

                                        intersection.z = *(intersections.begin());
                                        intersections.erase(intersections.begin());
				
					correlation_sample.m_intersection_points.insert( intersection );
				
					// pythagorus	
					double distance = sqrt( ((intersection.x - sample_centroid[0]) * (intersection.x - sample_centroid[0])) +
								((intersection.y - sample_centroid[1]) * (intersection.y - sample_centroid[1])) );

					// Now scale the distance up or down so that it's proportional to the
					// reference object's bounding box.  We've decided the scales are
					// close enough to be acceptable.  The distance valus are really
					// designed to compare shapes rather than sizes.  Make the two similar.
					distance *= required_scaling;

					// printf("Adding scaled distance of '%lf'\n", distance );

					correlation_sample.m_intersection_distances.insert( distance );
                                } // End while

				results.insert( std::make_pair( angle, correlation_sample ) );
                        } // End if - then
			else
			{
				// It didn't intersect it.  We need to remember this.

				// printf("It didn't intersect so we're adding an empty intersections array\n");
				CorrelationSample_t empty;
				results.insert( std::make_pair( angle, empty ) );
			} // End if - else

			heekscad_interface.DeleteUndoably(verification_line);
			verification_line = NULL;
		} // End if - then
		else
		{
			// printf("Failed to create intersection line\n");
		} // End if - else
	} // End for

	return(results);

} // End CorrelationPoints() method



// Convert the list of symbols into a list of reference coordinates.
std::set<CCorrelationTool::Point3d> CCorrelationTool::ReferencePoints( const CCorrelationTool::Symbols_t & sample_symbols ) const
{
	std::set<CCorrelationTool::Point3d> results;

	for (Symbols_t::const_iterator symbol = sample_symbols.begin(); symbol != sample_symbols.end(); symbol++)
	{
		HeeksObj *ob = heekscad_interface.GetIDObject( symbol->first, symbol->second );
		CBox box;
		ob->GetBox(box);
		double centroid[3];
		box.Centre(centroid);

		results.insert( Point3d( centroid[0], centroid[1], centroid[2] ) );
	} // End for

	// Run through these symbols and return the centroid of the bounding box.
	return(results);

} // End ReferencePoints() method



/**
	Find a 'score' (represented as a percentage) that represents how similar to two sets
	of correlation data are.
 */
double CCorrelationTool::Score( const CCorrelationTool::CorrelationData_t & sample, const CCorrelationTool::CorrelationData_t & reference ) const
{
	double distance_score = 0.0;
	double intersections_score = 0.0;

	unsigned int num_distance_samples = 0;

	// printf("Score() We have %d sample and %d reference points\n", sample.size(), reference.size() );

	if (sample.size() == 0)
	{
		return(0.0);
	} // End if - then

	if (reference.size() == 0)
	{
		return(0.0);
	} // End if - then

	if (sample.size() != reference.size())
	{
		return(0.0);	// This should not occur.
	} // End if - then

	CorrelationData_t::const_iterator l_itSample;
	CorrelationData_t::const_iterator l_itReference;

	for (l_itSample = sample.begin(), l_itReference = reference.begin(); (l_itSample != sample.end()) && (l_itReference != reference.end()); l_itSample++, l_itReference++)
	{
		if ((l_itSample->second.m_intersection_points.size() == 0) && (l_itReference->second.m_intersection_points.size() > 0))
		{
			// The reference has something at this angle but the sample doesn't
			intersections_score += 0.0;	// Nothing in common
		} // End if - then

		if ((l_itSample->second.m_intersection_points.size() > 0) && (l_itReference->second.m_intersection_points.size() == 0))
		{
			// The sample has something at this angle but the reference doesn't
			intersections_score += 0.0;	// Nothing in common
		} // End if - then

		if ((l_itSample->second.m_intersection_points.size() == 0) && (l_itReference->second.m_intersection_points.size() == 0))
		{
			// The same for both.
			intersections_score += 1.0;
		} // End if - then
		else
		{
			if (l_itSample->second.m_intersection_points.size() == l_itReference->second.m_intersection_points.size())
			{
				// The same for both.
				intersections_score += 1.0;
			} // End if - then
		} // End if - else

		// Well the number of intersection points is close enough.  Look through the distances and see if they're also
		// within scope.  Specifically, figure out what percentage each angle's distance differs by.  Create an average for this
		// difference set.  If the average difference in size is less than the min_correlation_factor then it's still good enough.

		std::set<double>::const_iterator l_itSampleDistance;
		std::set<double>::const_iterator l_itReferenceDistance;

		/*
		printf(" We have %d sample distances and %d reference distances\n",
			l_itSample->second.m_intersection_distances.size(),
			l_itReference->second.m_intersection_distances.size() );
		*/

		for (	l_itSampleDistance = l_itSample->second.m_intersection_distances.begin(),
			l_itReferenceDistance = l_itReference->second.m_intersection_distances.begin();
			l_itSampleDistance != l_itSample->second.m_intersection_distances.end() &&
			l_itReferenceDistance != l_itReference->second.m_intersection_distances.end();
			l_itSampleDistance++,
			l_itReferenceDistance++)
		{
			// printf("sample_dist='%lf', ref_dist='%lf'\n", *l_itSampleDistance, *l_itReferenceDistance );

			if (*l_itSampleDistance < *l_itReferenceDistance)
			{
				distance_score += double( double(*l_itSampleDistance) / double(*l_itReferenceDistance));
				// printf("increasing score by %lf\n", double( double(*l_itSampleDistance) / double(*l_itReferenceDistance)) );
				num_distance_samples++;
			} // End if - then
			else
			{
				distance_score += double( double(*l_itReferenceDistance) / double(*l_itSampleDistance));
				// printf("increasing score by %lf\n", double( double(*l_itReferenceDistance) / double(*l_itSampleDistance)) );
				num_distance_samples++;
			} // End if - else
		} // End for
	} // End for

	// Return the average score for all tests
	double average_distance_score = 0.0;

	if (num_distance_samples > 0)
	{
		average_distance_score = double(double(distance_score) / double(num_distance_samples));
	} // End if - then

	double average_intersections_score = 0.0;
	if (reference.size() > 0)
	{
		average_intersections_score = double(double(intersections_score) / double(reference.size()));
	} // End if - then

	double score = double((average_distance_score + average_intersections_score) / 2.0);

	// printf("average_distance_score='%lf'\n", average_distance_score);
	// printf("average_intersections_score='%lf'\n", average_intersections_score);
	
	// printf("Returning score=%lf\n", score);

	return(score);

} // End Score() method


/**
	- obtain the bounding box for both the reference and the sample objects.
	- scale the reference object up/down (with a maximum of m_max_scale_threshold) until
	  both objects are of a similar scale.  If this scaling cannot make their bounding boxes similar
	  enough within the configured maximum threshold then the sample object is discarded as 'not a match'.
	- If the scaling process can produce a similar bounding box, the application draws a logical line
	  from the centroid of each object (reference and sample) at a range of angles (eg: every 'n' degrees
	  around the full 360 circle).  It then intersects this line with the object to find both how many
	  intersections there are and what distance is found from the centroid to the intersection point(s).
	  The difference in these distances for both the reference and sample objects will be combined to
	  produce a correlation factor (score).  If this factor is within the m_min_correlation_factor
	  then the sample object's centroid is added to this object's reference points.
 */
CCorrelationTool::Symbols_t CCorrelationTool::SimilarSymbols( const CCorrelationTool::Symbol_t & reference_symbol ) const
{
	CCorrelationTool::Symbols_t result_set;
	std::set<CCorrelationTool::Point3d> reference_points;

	HeeksObj *pReference = heekscad_interface.GetIDObject( reference_symbol.first, reference_symbol.second );
	if (! pReference)
	{
		// Can't find the reference object.  Return an empty set.
		return(result_set);
	} // End if - then

	CorrelationData_t reference_correlation_data = CorrelationData( reference_symbol, reference_symbol, m_number_of_sample_points, m_max_scale_threshold );

	// Scan through all objects looking for something that's like this one.
	for (HeeksObj *ob = heekscad_interface.GetFirstObject(); ob != NULL; ob = heekscad_interface.GetNextObject())
	{
		if ((ob->GetType() == m_reference_symbol.first) && (ob->m_id == m_reference_symbol.second))
		{
			continue;	// It's the reference object.  They're identicle.
		} // End if - then

		CorrelationData_t sample_correlation_data = CorrelationData(	Symbol_t( ob->GetType(), ob->m_id ),
										reference_symbol,
                                               					m_number_of_sample_points,
                                               					m_max_scale_threshold );

		// Now compare the correlation data for both the reference and sample objects to see if we
		// think they're similar.

		if (Score( sample_correlation_data, reference_correlation_data ) >= m_min_correlation_factor)
		{
			result_set.push_back( Symbol_t( ob->GetType(), ob->m_id ) );
		} // End if - then
	} // End for

	return(result_set);

} // End SimilarSimbols() method





/**
 * 	This method looks through the symbols in the list.  If they're PointType objects
 * 	then the object's location is added to the result set.  If it's a circle object
 * 	that doesn't intersect any other element (selected) then add its centre to
 * 	the result set.  Finally, find the intersections of all of these elements and
 * 	add the intersection points to the result set.  We use std::set<Point3d> so that
 * 	we end up with no duplicate points.
 */
std::set<CCorrelationTool::Point3d> CCorrelationTool::FindAllLocations() const
{
	Symbols_t similar_symbols = SimilarSymbols( m_reference_symbol );
	return(ReferencePoints( similar_symbols ));

} // End FindAllLocations() method

