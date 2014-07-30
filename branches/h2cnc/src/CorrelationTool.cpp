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
CCorrelationTool::CorrelationData_t CCorrelationTool::CorrelationData(      HeeksObj *sample_object,
                                                HeeksObj *reference_object,
                                                const int number_of_sample_points,
                                                const double max_scale_threshold,
                                                const bool correlate_by_color ) const
{
	CorrelationData_t results;
	double required_scaling = 1.0;	// How much do we need to scale the sample object to make it the same
					// size as the reference object?  We need this value so that we can
					// scale the distance values as well.

	// Get each symbol's bounding box.
	CBox reference_box, sample_box;
	reference_object->GetBox(reference_box);
	sample_object->GetBox(sample_box);

	// First see if they're close enough in size to be a possible match.
	if (! SimilarScale( reference_box, sample_box, max_scale_threshold, &required_scaling ))
	{
		// They're too different in size.
		return(results);	// return an empty data set.
	} // End if - then

	if ((correlate_by_color) && (! ColorsMatch(reference_object, sample_object)))
    {
		return(results);	// return an empty data set.
	} // End if - then

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
		double verification_line_end[3] = { 	(cos( theta ) * radius) + sample_centroid[0],
							(sin( theta ) * radius) + sample_centroid[1],
							sample_centroid[2] };

		// Now find all intersection points between this verification line and the sample object.
		HeeksObj* verification_line = heekscad_interface.NewLine(sample_centroid, verification_line_end);
		if (verification_line)
		{
			std::list<double> intersections;
			if (sample_object->Intersects( verification_line, &intersections ))
			{
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

					correlation_sample.m_intersection_distances.insert( distance );
				} // End while

				results.insert( std::make_pair( angle, correlation_sample ) );
			} // End if - then
			else
			{
				// It didn't intersect it.  We need to remember this.

				CorrelationSample_t empty;
				results.insert( std::make_pair( angle, empty ) );
			} // End if - else

			wxGetApp().Remove(verification_line);
			verification_line = NULL;
		} // End if - then
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
		if (ob != NULL)
		{
			CBox box;
			ob->GetBox(box);
			double centroid[3];
			box.Centre(centroid);

			results.insert( Point3d( centroid[0], centroid[1], centroid[2] ) );
		} // End if - then
	} // End for

	// Run through these symbols and return the centroid of the bounding box.
	return(results);

} // End ReferencePoints() method



/**
	Find a 'score' (represented as a percentage) that represents how similar to two sets
	of correlation data are.

	There are some types of elements for which the Intersects() method does not work.  Rather
	than discarding all these objects, allow them to be selected/rejected based on size alone.
	This 'hit rate' will improve as the Intersects() method is expanded to work with more
	base data types.
 */
double CCorrelationTool::Score( const CCorrelationTool::CorrelationData_t & sample, const CCorrelationTool::CorrelationData_t & reference ) const
{
	double distance_score = 0.0;
	double intersections_score = 0.0;

	unsigned int num_distance_samples = 0;

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

	for (	l_itSample = sample.begin(), l_itReference = reference.begin();
		(l_itSample != sample.end()) && (l_itReference != reference.end());
		l_itSample++, l_itReference++)
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

		// Well, we know the intersections_score.  Look through the distances and see if they're also
		// within scope.  Specifically, figure out what percentage each angle's distance differs by.  Create an average for this
		// difference set.  If the average difference in size is less than the min_correlation_factor then it's still good enough.

		std::set<double>::const_iterator l_itSampleDistance;
		std::set<double>::const_iterator l_itReferenceDistance;

		if ((l_itSample->second.m_intersection_distances.size() == 0) ||
		    (l_itReference->second.m_intersection_distances.size() == 0))
		{
			// Either the sample or reference object doesn't seem to work with the Intersections()
			// method.  This is true of some data types.  In this case we won't discard the
			// element now.  Instead, we will give it a 'perfect' distances score so that
			// the element is selected/rejected based on size alone.

			num_distance_samples++;
			distance_score += 1.0;
		} // End if - then
		else
		{
			for (	l_itSampleDistance = l_itSample->second.m_intersection_distances.begin(),
				l_itReferenceDistance = l_itReference->second.m_intersection_distances.begin();
				l_itSampleDistance != l_itSample->second.m_intersection_distances.end() &&
				l_itReferenceDistance != l_itReference->second.m_intersection_distances.end();
				l_itSampleDistance++,
				l_itReferenceDistance++)
			{
				if (*l_itSampleDistance < *l_itReferenceDistance)
				{
					// The sample is shorter.  By how much?
					distance_score += double( double(*l_itSampleDistance) / double(*l_itReferenceDistance));
					num_distance_samples++;
				} // End if - then
				else
				{
					// The sample is longer.  By how much?
					distance_score += double( double(*l_itReferenceDistance) / double(*l_itSampleDistance));
					num_distance_samples++;
				} // End if - else
			} // End for
		} // End if - else
	} // End for

	// Return the average score for all tests
	double average_distance_score = 1.0;

	if (num_distance_samples > 0)
	{
		average_distance_score = double(double(distance_score) / double(num_distance_samples));
	} // End if - then

	double average_intersections_score = 1.0;
	if (reference.size() > 0)
	{
		average_intersections_score = double(double(intersections_score) / double(reference.size()));
	} // End if - then

	double score = double((average_distance_score + average_intersections_score) / 2.0);

	return(score);

} // End Score() method


/**
    Recursively retrieve all objects for this parent.
 */
std::list<HeeksObj *> CCorrelationTool::ListAllChildren( HeeksObj *parent ) const
{
    std::list<HeeksObj *> results;

    std::list<HeeksObj *> children;
    for (HeeksObj *child = parent->GetFirstChild(); child != NULL; child = parent->GetNextChild())
    {
        children.push_back(child);
    }

    for (std::list<HeeksObj *>::iterator itChild = children.begin(); itChild != children.end(); itChild++)
    {
        std::list<HeeksObj *> their_children = ListAllChildren(*itChild);
        for (std::list<HeeksObj *>::iterator itObj = their_children.begin(); itObj != their_children.end(); itObj++)
        {
            if (std::find(results.begin(), results.end(), *itObj) == results.end())
            {
                results.push_back(*itObj);
            }
        }
    }

    if (std::find(results.begin(), results.end(), parent) == results.end())
    {
        results.push_back(parent);
    }

    return(results);
}



bool CCorrelationTool::ColorsMatch( HeeksObj *obj1, HeeksObj *obj2 ) const
{
    if (obj1 == NULL) return(true);
    if (obj2 == NULL) return(true);

    const HeeksColor *pObj1Color = obj1->GetColor();
    const HeeksColor *pObj2Color = obj2->GetColor();

    if (pObj1Color == NULL) return(true);
    if (pObj2Color == NULL) return(true);

    return(*pObj1Color == *pObj2Color);
}


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
std::list<HeeksObj *> CCorrelationTool::SimilarSymbols( HeeksObj *pReference ) const
{
	std::list<HeeksObj *> result_set;
	std::set<CCorrelationTool::Point3d> reference_points;

	std::list<HeeksObj *> all_objects;
	for (HeeksObj *ob = heekscad_interface.GetFirstObject(); ob != NULL; ob = heekscad_interface.GetNextObject())
    {
        std::list<HeeksObj *> these_objects = CCorrelationTool::ListAllChildren( ob );
        for (std::list<HeeksObj *>::iterator itThisObj = these_objects.begin(); itThisObj != these_objects.end(); itThisObj++)
        {
            if (std::find(all_objects.begin(), all_objects.end(), *itThisObj) == all_objects.end())
            {
                all_objects.push_back(*itThisObj);
            }
        } // End for
    } // End for

	if (pReference->GetType() == PointType)
	{
		// The operator has selected a 'point' object to use as a reference.  The bounding box
		// and line intersection functionality isn't relevant for a point feature.  By the same
		// token, all points are 'equal'.  I can't think of a scenario where one point shouldn't
		// be considered equal to another point.  To that end, select all point objects in the
		// data model.

		for (std::list<HeeksObj *>::iterator itObject = all_objects.begin(); itObject != all_objects.end(); itObject++)
		{
		    HeeksObj *ob = *itObject;
			if (ob->GetType() == PointType)
			{
			    if (((m_correlate_by_color) && (ColorsMatch(pReference, ob))) || (m_correlate_by_color == false))
			    {
                    result_set.push_back( ob );
			    }
			} // End if - then
		} // End for

		return(result_set);
	} // End if - then

	CorrelationData_t reference_correlation_data = CorrelationData( pReference, pReference, m_number_of_sample_points, m_max_scale_threshold, m_correlate_by_color );

	// Scan through all objects looking for something that's like this one.
	for (std::list<HeeksObj *>::iterator itObject = all_objects.begin(); itObject != all_objects.end(); itObject++)
    {
        HeeksObj *ob = *itObject;

		if ((ob->GetType() == pReference->GetType()) && (ob->m_id == pReference->m_id))
		{
			// Include this in the result set.  Otherwise we would be selecting everything except what
			// we pointed to.
			result_set.push_back( pReference );
			continue;	// It's the reference object.  They're identicle.
		} // End if - then

		CorrelationData_t sample_correlation_data = CorrelationData( ob,
																	pReference,
																	m_number_of_sample_points,
																	m_max_scale_threshold,
																	m_correlate_by_color );

		// Now compare the correlation data for both the reference and sample objects to see if we
		// think they're similar.

		if (Score( sample_correlation_data, reference_correlation_data ) >= m_min_correlation_factor)
		{
		    if (((m_correlate_by_color) && (ColorsMatch(pReference, ob))) || (m_correlate_by_color == false))
		    {
                result_set.push_back( ob );
		    }
		} // End if - then
	} // End for

	return(result_set);

} // End SimilarSimbols() method




